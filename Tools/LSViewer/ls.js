"use strict";
/* ============================================================================
 *  Loader_Sequence engine for LSViewer.
 *
 *  A faithful JS port of the reversed Glacier loader-sequence pipeline so the
 *  sequence can be previewed offline (no engine).  Reference sources:
 *
 *    Player/flow : Glacier/source/LoaderSequence/ZLoader_Sequence_Wintel_D3D.cpp
 *                  (Start 0x4AF0B0, Get_Geoms_Data 0x4ADC50, Get_My_Data 0x4ADD60,
 *                   InstallTextureBuffer 0x4AE3D0)
 *    Parsing     : ZLoader_Sequence_Script_Reader.cpp (0x467C70, ...)
 *    Evaluation  : ZLoader_Sequence_Script.cpp (Adjust_Script_Time, Get_*, ...)
 *    Textures    : Glacier/source/Render/Bitmap/ZBitmap*.cpp
 *
 *  Kept intentionally close to the C++ so float parity can be checked with the
 *  existing gtest vectors.
 * ========================================================================== */

/* ----------------------------- helpers ---------------------------------- */
function DataViewLE(bytes) {
    return new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
}
function align4(x) { return (x + 3) & ~3; }
// Mirrors the engine walk: (strlen + 4) & ~3 (identical to align4(strlen + 1)).
function align4Name(strlen) { return (strlen + 4) & ~3; }
function stricmp(a, b) {
    return String(a).toLowerCase() === String(b).toLowerCase() ? 0 : 1;
}

// Constants from ZLoader_Sequence_Wintel_D3D.cpp / ZTextureType.h.
const kCompiled_Geom_Offset_Mask = 0x00FFFFFF;
const kLoader_Sequence_Setup_Type = 0x20011B;
const kSprite_Record_Type = 2;
const kPrim_Element_Floats = 10;
const kCube_Texture_Flag = 0x400;

const BITMAP = {
    PAL:      0x50414C4E,
    PAL_OPAC: 0x50414C4F,
    RGB32:    0x52474241,
    U8V8:     0x55385638,
    DXT1:     0x44585431,
    DXT3:     0x44585433,
    I8:       0x49382020,
};

/* ------------------------- ZIP container (M1) --------------------------- */
// Standard ZIP: walk the EOCD, then the central directory, inflating each
// local entry (raw deflate, wbits = -15).  Layout matches FsZip_t/ZIP.h.
function zipParse(bytes) {
    const view = DataViewLE(bytes);
    // Scan backwards for the End Of Central Directory signature 0x06054b50.
    let eocd = -1;
    for (let i = bytes.length - 22; i >= 0; --i) {
        if (view.getUint32(i, true) === 0x06054b50) { eocd = i; break; }
    }
    if (eocd < 0) throw new Error("Not a ZIP (no end-of-central-directory)");
    const numEntries = view.getUint16(eocd + 10, true);
    let o = view.getUint32(eocd + 16, true);

    const members = [];
    for (let n = 0; n < numEntries; ++n) {
        if (view.getUint32(o, true) !== 0x02014b50) throw new Error("Bad central-directory entry");
        const method = view.getUint16(o + 10, true);
        const csize = view.getUint32(o + 20, true);
        const fnlen = view.getUint16(o + 28, true);
        const extralen = view.getUint16(o + 30, true);
        const commentlen = view.getUint16(o + 32, true);
        const lho = view.getUint32(o + 42, true);
        const name = new TextDecoder("latin1").decode(bytes.subarray(o + 46, o + 46 + fnlen));
        members.push({ name, method, csize, lho });
        o += 46 + fnlen + extralen + commentlen;
    }
    return members;
}

function zipExtract(bytes, member) {
    const view = DataViewLE(bytes);
    if (view.getUint32(member.lho, true) !== 0x04034b50)
        throw new Error("Bad local file header for " + member.name);
    const fnlen = view.getUint16(member.lho + 26, true);
    const extralen = view.getUint16(member.lho + 28, true);
    const dataOff = member.lho + 30 + fnlen + extralen;
    const raw = bytes.subarray(dataOff, dataOff + member.csize);
    if (member.method === 8) return new Uint8Array(pako.inflate(raw, { raw: true }));
    if (member.method === 0) return raw;
    throw new Error("Unsupported ZIP compression method " + member.method);
}

// Engine member patterns are "*Loader_Sequence.<ext>"; match by suffix,
// case-insensitive (the engine FindFile uses the wildcard + IOFS comparison).
function zipFind(members, suffix) {
    suffix = suffix.toLowerCase();
    for (const m of members) {
        if (m.name.toLowerCase().endsWith(suffix)) return m;
    }
    return null;
}

/* ------------------- ZPackedDataChunk unpack (M2) ----------------------- */
// 9-byte stream header (iRaw_Size, iPacked_Size, eCompression) then either a
// raw-deflate payload (ZIPPED) or a raw copy (UNCOMPRESSED).
function unpackPackedDataChunk(bytes) {
    if (bytes.length < 9) throw new Error("GMS too small for a packed header");
    const view = DataViewLE(bytes);
    const rawSize = view.getInt32(0, true);
    const compression = bytes[8];
    const payload = bytes.subarray(9);
    if (compression === 1) {
        return payload.subarray(0, rawSize > 0 ? rawSize : payload.length);
    }
    if (compression === 0) {
        return new Uint8Array(pako.inflate(payload, { raw: true }));
    }
    throw new Error("Unknown ZPackedDataChunk compression " + compression);
}

/* ---------------- entity locate: Get_My_Data (M2) ----------------------- */
// SPackedGeomsHeader -> entity table {u32 count; SPackedGeomsTree[]} ->
// SCompiledGeom; return lPrim of the first ZLoader_Sequence_Setup.
function findLoaderSequencePrim(gms, prm) {
    const gv = DataViewLE(gms);
    const lEntriesOffset = gv.getUint32(0, true);
    const total = gv.getUint32(lEntriesOffset, true);
    for (let i = 0; i < total; ++i) {
        const tree = lEntriesOffset + 4 + 8 * i;
        const lcgo = gv.getUint32(tree, true);
        const cg = 4 * (lcgo & kCompiled_Geom_Offset_Mask);
        if (cg + 0x18 > gms.length) break;
        const geomType = gv.getUint32(cg + 0x14, true); // SCompiledGeom::lGeomType
        if (geomType === kLoader_Sequence_Setup_Type) {
            const lPrim = gv.getUint32(cg + 0x0C, true); // SCompiledGeom::lPrim
            return prm.subarray(lPrim);
        }
    }
    return null;
}

// Reads a NUL-terminated latin1 string starting at off; returns {text, end}.
function readCString(bytes, off) {
    let end = off;
    while (end < bytes.length && bytes[end] !== 0) ++end;
    return { text: new TextDecoder("latin1").decode(bytes.subarray(off, end)), end };
}

/* --------------------- minimal SimpleXML (M3) --------------------------- */
// Fires startElement(name, attrs) in document order and a no-op endElement.
// Quoted attribute values are required (Position carries a comma), matching the
// engine scanner described in the gtest header comment.
function xmlParse(text, startElement) {
    const n = text.length;
    let i = 0;
    while (i < n) {
        const lt = text.indexOf("<", i);
        if (lt < 0) break;
        let j = lt + 1;
        if (j >= n) break;
        // Skip comments, declarations, CDATA and processing instructions.
        if (text[j] === "!") {
            const close = text.indexOf(">", j);
            i = close < 0 ? n : close + 1;
            continue;
        }
        if (text[j] === "?") {
            const close = text.indexOf(">", j);
            i = close < 0 ? n : close + 1;
            continue;
        }
        // Closing tag </name>: endElement is a no-op in the reader, so skip it.
        if (text[j] === "/") {
            const close = text.indexOf(">", j);
            i = close < 0 ? n : close + 1;
            continue;
        }
        // Element name.
        let k = j;
        while (k < n && !/[\s/>]/.test(text[k])) ++k;
        const name = text.slice(j, k);
        const attrs = [];
        let selfClose = false;
        // Attributes.
        while (k < n) {
            while (k < n && /\s/.test(text[k])) ++k;
            if (k >= n) break;
            if (text[k] === ">") { ++k; break; }
            if (text[k] === "/" && text[k + 1] === ">") { selfClose = true; k += 2; break; }
            // Attribute name.
            let a0 = k;
            while (k < n && !/[\s=/>]/.test(text[k])) ++k;
            const aname = text.slice(a0, k);
            while (k < n && /\s/.test(text[k])) ++k;
            let aval = "";
            if (text[k] === "=") {
                ++k;
                while (k < n && /\s/.test(text[k])) ++k;
                const q = text[k];
                if (q === '"' || q === "'") {
                    ++k;
                    let v0 = k;
                    while (k < n && text[k] !== q) ++k;
                    aval = text.slice(v0, k);
                    if (k < n) ++k;
                } else {
                    let v0 = k;
                    while (k < n && !/[\s>]/.test(text[k])) ++k;
                    aval = text.slice(v0, k);
                }
            }
            if (aname) attrs.push([aname, aval]);
        }
        startElement(name, attrs);
        i = k;
        if (selfClose) continue;
    }
}
function xmlGetAttr(attrs, name) {
    for (const [k, v] of attrs) if (k === name) return v;
    return null;
}

/* ---------------- script reader (port of Script_Reader) (M3) ------------- */
const ROOT_ELEMENT = "Loader_Sequence_Script";
const KEY_FRAME_ELEMENT = "Key_Frame";
const PICTURE_SETTINGS_ELEMENT = "Picture_Settings";
const SCREEN_ELEMENT = "Screen";
const NO_TIME = -10.0;
const UNSET_BITS = 0xFFFFFFFF;

// Reads one float, matching the engine's `static_cast<float>(atof(...))`.
function atof(s) { return parseFloat(s) || 0; }

// Splits "X,Y" into two floats; a missing comma duplicates the value (Get_Cds).
function getCds(s) {
    const comma = s.indexOf(",");
    if (comma < 0) {
        const v = atof(s);
        return { x: v, y: v };
    }
    return { x: atof(s.slice(0, comma)), y: atof(s.slice(comma + 1)) };
}

// Unset float representation: the engine stores 0xFFFFFFFF bit pattern (NaN).
// We model "unset" as JS null and reproduce the same fill/interpolate passes.
function loadScriptInfo(xmlText) {
    let readMode = "gather"; // gather | fill
    let nrKeyFrames = 0;
    let currentKeyFrameNr = 0;
    let current = -1.0;
    let lastKeyFrameTime = 0.0;
    let screenSizeX = 640.0, screenSizeY = 400.0;
    let fullProgressTime = -1.0;

    // Picture names (deduped case-insensitively, order preserved).
    const picNames = [];
    function addPictureName(name) {
        for (const p of picNames) if (stricmp(p, name) === 0) return;
        picNames.push(name);
    }
    function getPictureNr(name) {
        for (let i = 0; i < picNames.length; ++i) if (stricmp(picNames[i], name) === 0) return i;
        return 0;
    }

    const keyTimes = [];          // fill pass
    let picTable = [];             // flat array [pic + kf*nrPics]
    let nrPics = 0;

    // -------- gather pass --------
    readMode = "gather"; current = -1.0;
    xmlParse(xmlText, (name, attrs) => {
        if (name === ROOT_ELEMENT) return;
        if (name === KEY_FRAME_ELEMENT) {
            const t = xmlGetAttr(attrs, "Time");
            if (t !== null) {
                const fTime = atof(t);
                current = fTime;
                if (nrKeyFrames !== 0 || fTime === 0.0) ++nrKeyFrames;
                else nrKeyFrames = 2; // implicit key frame at t=0
            } else {
                current = NO_TIME;
            }
            return;
        }
        if (name === PICTURE_SETTINGS_ELEMENT && current !== NO_TIME) {
            const p = xmlGetAttr(attrs, "Picture");
            if (p !== null) addPictureName(p);
        }
    });

    nrPics = picNames.length;
    const nrKeys = nrKeyFrames;

    // Allocate table with all fields unset (null), like memset 0xFF.
    picTable = new Array(nrKeys * nrPics);
    for (let i = 0; i < picTable.length; ++i)
        picTable[i] = { picNr: 0, posX: null, posY: null, opacity: null, multiply: null, interp: UNSET_BITS };
    for (let i = 0; i < nrKeys; ++i) keyTimes.push(0.0);

    // -------- fill pass --------
    readMode = "fill"; current = -1.0; currentKeyFrameNr = 0;
    xmlParse(xmlText, (name, attrs) => {
        if (name === ROOT_ELEMENT) return;
        if (name === KEY_FRAME_ELEMENT) {
            const t = xmlGetAttr(attrs, "Time");
            if (t === null) { current = NO_TIME; return; }
            const fTime = atof(t);
            current = fTime;
            if (fTime > lastKeyFrameTime) lastKeyFrameTime = fTime;
            if (currentKeyFrameNr === 0 && fTime !== 0.0) {
                keyTimes[currentKeyFrameNr++] = 0.0; // implicit zero key frame
            }
            if (currentKeyFrameNr < nrKeys) keyTimes[currentKeyFrameNr++] = fTime;
            return;
        }
        if (name === PICTURE_SETTINGS_ELEMENT) {
            if (current === NO_TIME) return;
            const p = xmlGetAttr(attrs, "Picture");
            if (p === null) return;
            const picNr = getPictureNr(p);
            const idx = picNr + nrPics * (currentKeyFrameNr - 1);
            if (idx < 0 || idx >= picTable.length) return;
            const e = picTable[idx];
            e.picNr = picNr;
            const pos = xmlGetAttr(attrs, "Position");
            if (pos !== null) { const c = getCds(pos); e.posX = c.x; e.posY = c.y; }
            const op = xmlGetAttr(attrs, "Opacity");
            if (op !== null) e.opacity = atof(op);
            const mu = xmlGetAttr(attrs, "Multiply");
            if (mu !== null) e.multiply = atof(mu);
            const ip = xmlGetAttr(attrs, "Position_Interpolation");
            if (ip !== null) {
                if (ip === "Time") e.interp = 0;
                else if (ip === "Progress") e.interp = 1;
            }
            return;
        }
        if (name === SCREEN_ELEMENT) {
            const ss = xmlGetAttr(attrs, "Screen_Size");
            if (ss !== null) { const c = getCds(ss); screenSizeX = c.x; screenSizeY = c.y; }
            const fp = xmlGetAttr(attrs, "Full_Progress_Time");
            if (fp !== null) fullProgressTime = atof(fp);
        }
    });

    fillKeysNotEntered();

    function fillKeysNotEntered() {
        if (nrKeys === 0 || nrPics === 0) return;
        if (fullProgressTime < 0.0) fullProgressTime = lastKeyFrameTime;

        for (let p = 0; p < nrPics; ++p) {
            const first = picTable[p];
            if (first.posX === null) first.posX = 0.0;
            if (first.posY === null) first.posY = 0.0;
            if (first.opacity === null) first.opacity = 1.0;
            if (first.multiply === null) first.multiply = 1.0;
            if (first.interp === UNSET_BITS) first.interp = 0;

            let interp = first.interp;
            for (let k = 0; k < nrKeys; ++k) {
                const e = picTable[p + k * nrPics];
                if (e.interp === UNSET_BITS) e.interp = interp;
                else interp = e.interp;
            }
            interpUndefinedColumn(p, "posX");
            interpUndefinedColumn(p, "posY");
            interpUndefinedColumn(p, "opacity");
            interpUndefinedColumn(p, "multiply");
        }
    }

    // Port of Interpolate_Undefined_Column: fill nulls between the surrounding
    // defined key frames, carry the last defined value forward past the last.
    function interpUndefinedColumn(iPictureNr, field) {
        let fLastValue = picTable[iPictureNr][field]; // row 0 already default-filled
        let fLastTime = 0.0;
        for (let k = 0; k < nrKeys; ++k) {
            const e = picTable[iPictureNr + k * nrPics];
            const rValue = e[field];
            const fCurTime = keyTimes[k];
            if (rValue !== null) { fLastValue = rValue; fLastTime = fCurTime; continue; }

            let foundNext = false, fNextValue = 0.0, fNextTime = 0.0;
            for (let m = k + 1; m < nrKeys; ++m) {
                const v = picTable[iPictureNr + m * nrPics][field];
                if (v !== null) { fNextValue = v; fNextTime = keyTimes[m]; foundNext = true; break; }
            }
            if (foundNext) {
                const fSpan = fNextTime - fLastTime;
                const r = ((fNextTime - fCurTime) * fLastValue + (fCurTime - fLastTime) * fNextValue) / fSpan;
                e[field] = r;
                fLastValue = r; fLastTime = fCurTime;
            } else {
                e[field] = fLastValue;
            }
        }
    }

    return {
        nrKeyFrames: nrKeys, nrPictures: nrPics, nrPics, picNames,
        keyTimes, table: picTable,
        screenSizeX, screenSizeY, fullProgressTime,
    };
}

/* ---------------- script evaluator (port of Script) (M3/M5) ------------ */
class LoaderSequence {
    constructor(info) {
        this.info = info;
        this.progress = 0.0;
        this.timeAdjustment = 0.0;
    }
    setProgress(p) { this.progress = p; }

    adjustScriptTime(t) {
        if (t >= this.info.fullProgressTime && this.progress < 1.0)
            this.timeAdjustment = t - this.info.fullProgressTime;
        return t - this.timeAdjustment;
    }
    endKeyFrame(t) {
        const n = this.info.nrKeyFrames;
        const keys = this.info.keyTimes;
        if (n === 0) return 0;
        if (t < 0.0) t = 0.0;
        if (keys[n - 1] < t) t = keys[n - 1];
        let i = 0;
        while (i < n && t >= keys[i]) ++i;
        return i === n ? n - 1 : i;
    }
    interpField(picNr, fAdjTime, field) {
        const nrPics = this.info.nrPics || this.info.nrPictures;
        const keys = this.info.keyTimes, table = this.info.table;
        const next = this.endKeyFrame(fAdjTime);
        let fPrevValue = 0.0, fPrevTime = 0.0;
        if (next !== 0) {
            fPrevValue = table[picNr + (next - 1) * nrPics][field];
            fPrevTime = keys[next - 1];
        }
        const fNextTime = keys[next];
        let t = fAdjTime;
        if (t < fPrevTime) t = fPrevTime;
        if (fNextTime < t) t = fNextTime;
        const denom = fNextTime - fPrevTime;
        const fNextValue = table[picNr + next * nrPics][field];
        if (denom === 0) return fNextValue;
        return ((t - fPrevTime) * fNextValue + (fNextTime - t) * fPrevValue) / denom;
    }
    getPosX(picNr, t) {
        let fAdj = this.adjustScriptTime(t);
        const nrPics = this.info.nrPictures;
        const keys = this.info.keyTimes;
        let first = this.endKeyFrame(0.0);
        if (first !== 0) first -= 1;
        if (this.info.table[picNr + first * nrPics].interp === 1 /* Progress */) {
            const last = this.endKeyFrame(1e38);
            fAdj = keys[last] * this.progress + (1.0 - this.progress) * keys[0];
        }
        return this.interpField(picNr, fAdj, "posX");
    }
    getPosY(picNr, t) { return this.interpField(picNr, this.adjustScriptTime(t), "posY"); }
    getOpacity(picNr, t) { return this.interpField(picNr, this.adjustScriptTime(t), "opacity"); }
    getMultiply(picNr, t) { return this.interpField(picNr, this.adjustScriptTime(t), "multiply"); }
}

/* ------------------------- bitmap decoding (M6) ------------------------- */
// Parses the ZBitmap::LoadBin header (base) plus the payload for a specific
// type and returns { width, height, params, rgba:Uint8ClampedArray }.
// The record layout (from the TEX buffer) is:
//   [u32 reserved][u32 type][ZBitmap::LoadBin payload ...]
function decodeZBitmap(bytes, recOff, type) {
    const v = DataViewLE(bytes);
    const p = recOff + 8; // skip the {reserved,type} record header
    const hdrType = v.getUint32(p, true);
    const sizeX = (v.getUint32(p + 8, true) >> 16) & 0xFFFF;
    const sizeY = v.getUint32(p + 8, true) & 0xFFFF;
    const mipCount = v.getUint32(p + 12, true);
    const params = v.getUint32(p + 16, true);

    // Walk past the name string to the mip-size list.
    let nameStart = p + 28;
    let q = nameStart;
    while (q < bytes.length && bytes[q] !== 0) ++q;
    let cur = q + 1;
    let dataOff = 0;
    for (let i = 0; i < mipCount; ++i) {
        const dwSize = v.getUint32(cur, true); cur += 4;
        if (i === 0) dataOff = cur;
        cur += dwSize;
    }
    const afterMips = cur;

    const rgba = new Uint8ClampedArray(sizeX * sizeY * 4);
    writePixels(bytes, v, dataOff, afterMips, type, sizeX, sizeY, rgba);
    return { width: sizeX, height: sizeY, params, rgba };
}

// Fill `rgba` (RGBA byte order, R in byte 0) for the given bitmap type,
// mirroring each ZBitmap::*::GetRGBA.  For DXT the engine decodes to B|G<<8|R<<16
// (see ZBitmapDXT1::GetData "BGRAToRGBA"), so we swap R/B back here.
function writePixels(bytes, v, d, after, type, w, h, rgba) {
    const put = (i, r, g, b, a) => {
        const o = i * 4; rgba[o] = r; rgba[o + 1] = g; rgba[o + 2] = b; rgba[o + 3] = a;
    };
    if (type === BITMAP.RGB32) {
        for (let i = 0; i < w * h; ++i) { const x = v.getUint32(d + 4 * i, true); put(i, x & 255, (x >> 8) & 255, (x >> 16) & 255, (x >>> 24) & 255); }
    } else if (type === BITMAP.PAL) {
        const palSize = v.getInt32(after, true);
        const palBase = after + 4;
        for (let i = 0; i < w * h; ++i) {
            const c = v.getUint32(palBase + 4 * bytes[d + i], true);
            put(i, c & 255, (c >> 8) & 255, (c >> 16) & 255, (c >>> 24) & 255);
        }
    } else if (type === BITMAP.PAL_OPAC) {
        const palSize = v.getInt32(after, true);
        const palBase = after + 4;
        const opacBase = palBase + 4 * palSize;
        for (let i = 0; i < w * h; ++i) {
            const c = v.getUint32(palBase + 4 * bytes[d + i], true) & 0xFFFFFF;
            put(i, c & 255, (c >> 8) & 255, (c >> 16) & 255, bytes[opacBase + i]);
        }
    } else if (type === BITMAP.I8) {
        for (let i = 0; i < w * h; ++i) { const x = bytes[d + i]; put(i, x, x, x, x); }
    } else if (type === BITMAP.U8V8) {
        for (let i = 0; i < w * h; ++i) {
            const s = v.getUint16(d + 2 * i, true);
            const c = ((s << 8) | 0xFF0000FF) >>> 0; // R=255,G=U,B=V,A=255 (R in LSB)
            put(i, c & 255, (c >> 8) & 255, (c >> 16) & 255, (c >>> 24) & 255);
        }
    } else if (type === BITMAP.DXT1 || type === BITMAP.DXT3) {
        decodeDXT(bytes, v, d, type, w, h, rgba, put);
    } else {
        // Unknown type: leave transparent (caller falls back to a placeholder).
    }
}

// Port of Decode4x4DXT_RGB / Decode4x4DXT3 into the padded (w+3)&~3 grid, then
// crop to w×h and swap B/R (the engine decodes B|G<<8|R<<16|A<<24).
function decodeDXT(bytes, v, d, type, w, h, rgba, put) {
    const isD3 = type === BITMAP.DXT3;
    const blockSize = isD3 ? 16 : 8;
    const pw = (w + 3) & ~3, ph = (h + 3) & ~3;
    const grid = new Array(pw * ph);
    const blockRows = ((ph - 4) >> 2) + 1;
    const blockCols = ((pw - 4) >> 2) + 1;
    for (let by = 0; by < blockRows; ++by) {
        for (let bx = 0; bx < blockCols; ++bx) {
            const bo = d + (by * blockCols + bx) * blockSize;
            decodeBlock(bytes, v, bo, isD3, grid, pw, by * 4, bx * 4);
        }
    }
    for (let y = 0; y < h; ++y) {
        for (let x = 0; x < w; ++x) {
            const c = grid[y * pw + x];
            if (!c) { put(y * w + x, 0, 0, 0, 0); continue; }
            put(y * w + x, c[2], c[1], c[0], c[3]); // B,G,R,A -> R,G,B,A
        }
    }
}

function decodeBlock(bytes, v, o, isD3, grid, pw, oy, ox) {
    let cOff;
    let alphaLo = 0, alphaHi = 0;
    if (isD3) {
        alphaLo = v.getUint32(o, true); alphaHi = v.getUint32(o + 4, true);
        cOff = o + 8;
    } else {
        cOff = o;
    }
    const c0 = v.getUint16(cOff, true);
    const c1 = v.getUint16(cOff + 2, true);
    const r0 = (c0 >> 8) & 0xF8, g0 = (c0 >> 3) & 0xFC, b0 = (c0 & 0x1F) << 3;
    const r1 = (c1 >> 8) & 0xF8, g1 = (c1 >> 3) & 0xFC, b1 = (c1 & 0x1F) << 3;
    const pal = [];
    pal.push([b0, g0, r0]);
    pal.push([b1, g1, r1]);
    if (c0 <= c1) {
        pal.push([(b0 + b1) >> 1, (g0 + g1) >> 1, (r0 + r1) >> 1]);
        pal.push(isD3 ? [0, 0, 0] : [0, 0, 0]); // index 3
    } else {
        pal.push([((2 * b0 + b1) / 3) | 0, ((2 * g0 + g1) / 3) | 0, ((2 * r0 + r1) / 3) | 0]);
        pal.push([((b0 + 2 * b1) / 3) | 0, ((g0 + 2 * g1) / 3) | 0, ((r0 + 2 * r1) / 3) | 0]);
    }
    const idx = v.getUint32(o + (isD3 ? 12 : 4), true);
    // DXT1 three-color mode (c0 <= c1): palette index 3 is fully transparent.
    const threeColor = c0 <= c1;
    for (let i = 0; i < 16; ++i) {
        const sel = (idx >> (2 * i)) & 3;
        const c = pal[sel];
        let a;
        if (isD3) {
            const nib = i < 8 ? ((alphaLo >> (4 * (i & 7))) & 0xF) : ((alphaHi >> (4 * (i & 7))) & 0xF);
            a = nib * 0x11;
        } else {
            a = (threeColor && sel === 3) ? 0 : 255;
        }
        grid[(oy + (i >> 2)) * pw + (ox + (i & 3))] = [c[0], c[1], c[2], a];
    }
}

/* --------------------------- top-level load ----------------------------- */
// Loads a Loader_Sequence.ZIP ArrayBuffer and returns:
//   { xml, info, script, screen:{x,y}, fullProgressTime, pictures:[ ... ] }
async function loadLoaderSequence(buffer) {
    const bytes = new Uint8Array(buffer);
    const members = zipParse(bytes);

    const gmsM = zipFind(members, ".gms");
    const prmM = zipFind(members, ".prm");
    const texM = zipFind(members, ".tex");
    if (!gmsM) throw new Error("Missing *Loader_Sequence.GMS member");
    if (!prmM) throw new Error("Missing *Loader_Sequence.PRM member");
    if (!texM) throw new Error("Missing *Loader_Sequence.TEX member");

    const gmsRaw = zipExtract(bytes, gmsM);
    const gms = unpackPackedDataChunk(gmsRaw);
    const prm = zipExtract(bytes, prmM);
    const tex = zipExtract(bytes, texM);

    const scriptPrim = findLoaderSequencePrim(gms, prm);
    if (!scriptPrim) throw new Error("No ZLoader_Sequence_Setup entity found in GMS");

    // scriptPrim already points at prm+lPrim; the XML is the NUL-terminated
    // string there.
    const { text: xml, end: xmlEnd } = readCString(scriptPrim, 0);
    // Name blob begins at align4(strlen+4) past the string start. Compute its
    // absolute offset into `prm` (scriptPrim is a subarray of prm at lPrim).
    const strlen = xmlEnd; // index of NUL == string length
    const strStart = prm.length - scriptPrim.length;
    const blobAbs = strStart + align4Name(strlen);
    if (blobAbs + 4 > prm.length) throw new Error("PRM truncated before the name blob");

    const info = loadScriptInfo(xml);
    const script = new LoaderSequence(info);

    // ---- name blob walk (InstallTextureBuffer) ----
    const texTabOff = DataViewLE(tex).getUint32(0, true);
    const pictures = [];
    if (info.nrPictures > 0) {
        const nameCount = DataViewLE(prm).getUint32(blobAbs, true);
        let e = blobAbs + 4;
        const blob = [];
        for (let k = 0; k < nameCount; ++k) {
            const { text: nm, end: ne } = readCString(prm, e);
            const aligned = align4Name(nm.length);
            const hdr = e + aligned;
            const hv = DataViewLE(prm);
            const recOff = hv.getUint32(hdr, true);
            const subCount = hv.getUint32(hdr + 4, true);
            const subs = [];
            for (let j = 0; j < subCount; ++j) subs.push(hv.getUint32(hdr + 8 + 4 * j, true));
            blob.push({ nm, recOff, subs });
            e = hdr + 8 + 4 * subCount;
        }

        for (let p = 0; p < info.nrPictures; ++p) {
            const picName = info.picNames[p];
            const entry = blob.find(b => stricmp(b.nm, picName) === 0);
            const pic = { name: picName, sprites: [] };
            pictures.push(pic);
            if (!entry || entry.recOff === 0xFFFFFFFF || entry.recOff === -1 >>> 0) continue;

            const rec = entry.recOff;
            const elementCount = DataViewLE(prm).getUint32(rec, true);
            const elBase = rec + 4;
            for (let j = 0; j < entry.subs.length; ++j) {
                const so = entry.subs[j];
                if (so === 0) continue;
                const sv = DataViewLE(prm);
                const recType = sv.getUint16(so + 2, true);
                if (recType !== kSprite_Record_Type) continue;
                const texId = sv.getUint16(so + 4, true);
                const texOff = DataViewLE(tex).getUint32(texTabOff + 4 * texId, true);
                if (texOff === 0) continue;
                const recHdrType = DataViewLE(tex).getUint32(texOff + 4, true);

                // Element j floats. The ground-truth layout is
                //   [0]centerX [1]centerY ... [7]width [8]height
                // (Y is up in this space, so the canvas top-left flips centerY).
                // The element width/height match the decoded texture size.
                const fbase = elBase + 40 * j;
                const fv = DataViewLE(prm);
                const centerX = fv.getFloat32(fbase, true);
                const centerY = fv.getFloat32(fbase + 4, true);
                const elW = fv.getFloat32(fbase + 28, true);
                const elH = fv.getFloat32(fbase + 32, true);
                const posX = centerX - elW * 0.5;
                const posY = -centerY - elH * 0.5;

                let canvas = null, tw = 0, th = 0, vis = null;
                try {
                    const bm = decodeZBitmap(tex, texOff, recHdrType);
                    tw = bm.width; th = bm.height;
                    vis = alphaBounds(bm);   // sprite-local visible rect (for the gizmo)
                    if ((bm.params & kCube_Texture_Flag) === 0 && tw > 0 && th > 0 &&
                        typeof document !== "undefined") {
                        canvas = bitmapToCanvas(bm);
                    }
                } catch (err) { /* keep placeholder */ }
                pic.sprites.push({ canvas, texW: tw, texH: th, posX, posY, vis });
            }
        }
    }

    return {
        xml, info, script,
        screen: { x: info.screenSizeX, y: info.screenSizeY },
        fullProgressTime: info.fullProgressTime,
        pictures,
    };
}

// Bounding box of the non-transparent pixels (alpha > threshold) in sprite-local
// coordinates, or null when the sprite is fully transparent. Used to draw a
// tight gizmo around only the visible content (e.g. a text glyph) instead of the
// full-screen sprite quad.
function alphaBounds(bm, threshold) {
    if (threshold === undefined) threshold = 8;
    let minx = Infinity, miny = Infinity, maxx = -1, maxy = -1;
    const w = bm.width, h = bm.height, a = bm.rgba;
    for (let y = 0; y < h; ++y) {
        const row = y * w * 4;
        for (let x = 0; x < w; ++x) {
            if (a[row + x * 4 + 3] > threshold) {
                if (x < minx) minx = x; if (x > maxx) maxx = x;
                if (y < miny) miny = y; if (y > maxy) maxy = y;
            }
        }
    }
    if (maxx < 0) return null;
    return { x: minx, y: miny, w: maxx - minx + 1, h: maxy - miny + 1 };
}

// Builds a canvas from a decoded bitmap (RGBA bytes).
function bitmapToCanvas(bm) {
    const cv = document.createElement("canvas");
    cv.width = bm.width; cv.height = bm.height;
    const ctx = cv.getContext("2d");
    const img = ctx.createImageData(bm.width, bm.height);
    img.data.set(bm.rgba);
    ctx.putImageData(img, 0, 0);
    return cv;
}

// Exported for potential node-based testing.
if (typeof module !== "undefined") {
    module.exports = { loadLoaderSequence, loadScriptInfo, LoaderSequence, decodeZBitmap, alphaBounds, zipParse, zipExtract };
}
