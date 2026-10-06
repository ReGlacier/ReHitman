// Node harness (not shipped with the viewer): validates M2/M3/M6 parity.
global.pako = require("./pako.min.js");
global.module = undefined;
const fs = require("fs");
const vm = require("vm");

// Load ls.js into a shared global so its exports become reachable.
const src = fs.readFileSync("./ls.js", "utf8");
const sandbox = { pako: global.pako, document: undefined, module: undefined,
                  console, TextDecoder, DataView, Uint8Array, Uint8ClampedArray, Array,
                  Math, parseFloat, String, ArrayBuffer, require, setTimeout };
vm.createContext(sandbox);
vm.runInContext(src + "\n;globalThis.__exports = {loadLoaderSequence, loadScriptInfo, LoaderSequence, decodeZBitmap, alphaBounds};", sandbox);
const { loadLoaderSequence, loadScriptInfo } = sandbox.__exports;

let fails = 0;
function approx(a, b, eps) { return Math.abs(a - b) <= (eps || 1e-4); }
function check(name, cond) { if (!cond) { console.log("FAIL: " + name); fails++; } else console.log("ok:   " + name); }

// ---- gtest vector 1: CountsKeysPicturesAndInterpolatesUnsetColumns ----
{
    const xml = "<Loader_Sequence_Script>"
      + "<Key_Frame Time=\"0\"><Picture_Settings Picture=\"bg\" Position=\"10,20\" Opacity=\"0.5\" Multiply=\"2\" Position_Interpolation=\"Time\"/></Key_Frame>"
      + "<Key_Frame Time=\"1\"></Key_Frame>"
      + "<Key_Frame Time=\"2\"><Picture_Settings Picture=\"bg\" Position=\"100,20\"/></Key_Frame>"
      + "</Loader_Sequence_Script>";
    const info = loadScriptInfo(xml);
    check("gtest1 nrKeys=3", info.nrKeyFrames === 3);
    check("gtest1 nrPics=1", info.nrPictures === 1);
    check("gtest1 key0=0", approx(info.keyTimes[0], 0));
    check("gtest1 key1=1", approx(info.keyTimes[1], 1));
    check("gtest1 key2=2", approx(info.keyTimes[2], 2));
    const Pic = (p, k) => info.table[p + info.nrPics * k];
    check("gtest1 pic0k0 pos", approx(Pic(0,0).posX,10) && approx(Pic(0,0).posY,20) && approx(Pic(0,0).opacity,0.5) && approx(Pic(0,0).multiply,2));
    check("gtest1 interp mode Time", Pic(0,0).interp === 0);
    check("gtest1 pic0k1 posX=55", approx(Pic(0,1).posX, 55));
    check("gtest1 pic0k1 posY=20", approx(Pic(0,1).posY, 20));
    check("gtest1 pic0k1 op=0.5", approx(Pic(0,1).opacity,0.5));
    check("gtest1 pic0k1 mult=2", approx(Pic(0,1).multiply,2));
    check("gtest1 pic0k2 posX=100", approx(Pic(0,2).posX, 100));
}
// ---- gtest vector 2: InsertsImplicitKeyFrameAtZero ----
{
    const xml = "<Loader_Sequence_Script><Key_Frame Time=\"0.5\"><Picture_Settings Picture=\"logo\" Position=\"7,8\"/></Key_Frame></Loader_Sequence_Script>";
    const info = loadScriptInfo(xml);
    check("gtest2 nrKeys=2", info.nrKeyFrames === 2);
    check("gtest2 key0=0", approx(info.keyTimes[0], 0));
    check("gtest2 key1=0.5", approx(info.keyTimes[1], 0.5));
    const Pic = (p, k) => info.table[p + info.nrPics * k];
    check("gtest2 row0 pos=0", approx(Pic(0,0).posX,0) && approx(Pic(0,0).posY,0));
    check("gtest2 row0 opacity=1", approx(Pic(0,0).opacity,1));
    check("gtest2 row1 pos 7,8", approx(Pic(0,1).posX,7) && approx(Pic(0,1).posY,8));
    check("gtest2 screen defaults", approx(info.screenSizeX,640) && approx(info.screenSizeY,400));
    check("gtest2 fullprogress=0.5", approx(info.fullProgressTime,0.5));
}
// ---- gtest vector 3: ReadsScreenSizeAndFullProgressTime ----
{
    const xml = "<Loader_Sequence_Script><Screen Screen_Size=\"800,600\" Full_Progress_Time=\"5\"/><Key_Frame Time=\"1\"><Picture_Settings Picture=\"a\"/></Key_Frame></Loader_Sequence_Script>";
    const info = loadScriptInfo(xml);
    check("gtest3 screen 800x600", approx(info.screenSizeX,800) && approx(info.screenSizeY,600));
    check("gtest3 fullprogress=5", approx(info.fullProgressTime,5));
    check("gtest3 pic name a", info.picNames[0] === "a");
}
// ---- gtest vector 4: PositionWithoutCommaDuplicatesAxis ----
{
    const xml = "<Loader_Sequence_Script><Key_Frame Time=\"0\"><Picture_Settings Picture=\"p\" Position=\"42\"/></Key_Frame></Loader_Sequence_Script>";
    const info = loadScriptInfo(xml);
    check("gtest4 dup axis 42", approx(info.table[0].posX,42) && approx(info.table[0].posY,42));
}
// ---- gtest vector 5: PictureNamesDeduplicateCaseInsensitively ----
{
    const xml = "<Loader_Sequence_Script><Key_Frame Time=\"0\"><Picture_Settings Picture=\"bg\"/></Key_Frame><Key_Frame Time=\"1\"><Picture_Settings Picture=\"BG\"/></Key_Frame></Loader_Sequence_Script>";
    const info = loadScriptInfo(xml);
    check("gtest5 dedup=1", info.nrPictures === 1);
    check("gtest5 name bg", info.picNames[0] === "bg");
}
// ---- gtest vector 6: ProgressInterpolationModeIsParsed ----
{
    const xml = "<Loader_Sequence_Script><Key_Frame Time=\"0\"><Picture_Settings Picture=\"p\" Position_Interpolation=\"Progress\"/></Key_Frame><Key_Frame Time=\"1\"><Picture_Settings Picture=\"p\" Position_Interpolation=\"Time\"/></Key_Frame></Loader_Sequence_Script>";
    const info = loadScriptInfo(xml);
    const Pic = (p, k) => info.table[p + info.nrPics * k];
    check("gtest6 k0 progress=1", Pic(0,0).interp === 1);
    check("gtest6 k1 time=0", Pic(0,1).interp === 0);
}

// ---- bitmap decoder unit tests (M6) ----
{
    const { decodeZBitmap, alphaBounds } = sandbox.__exports;
    // Builds a TEX record: {u32 reserved, u32 type} + ZBitmap::LoadBin header +
    // mip data + format-specific tail.
    function makeRecord(type, w, h, mips, tail) {
        const parts = [];
        const u32a = (v) => { const b = new Uint8Array(4); new DataView(b.buffer).setUint32(0, v >>> 0, true); return b; };
        const f32 = (v) => { const b = new Uint8Array(4); new DataView(b.buffer).setFloat32(0, v, true); return b; };
        parts.push(u32a(0));                 // reserved
        parts.push(u32a(type));               // record type
        parts.push(u32a(type));               // header type (must match)
        parts.push(u32a(0));                  // id
        parts.push(u32a(((w & 0xFFFF) << 16) | (h & 0xFFFF)));
        parts.push(u32a(1));                  // mip count
        parts.push(u32a(0));                  // params
        parts.push(f32(1.0));                 // scale
        parts.push(u32a(0));                  // checksum
        parts.push(new Uint8Array(1));        // empty name + NUL
        for (const m of mips) { parts.push(u32a(m.length)); parts.push(m); }
        if (tail) parts.push(tail);
        let len = 0; for (const p of parts) len += p.length;
        const out = new Uint8Array(len); let o = 0;
        for (const p of parts) { out.set(p, o); o += p.length; }
        return out;
    }
    const palBytes = (arr) => { const b = new Uint8Array(4 + 4 * arr.length); const v = new DataView(b.buffer);
        v.setInt32(0, arr.length, true); arr.forEach((c, i) => v.setUint32(4 + 4 * i, c >>> 0, true)); return b; };
    const px = (bm, i) => [bm.rgba[i*4], bm.rgba[i*4+1], bm.rgba[i*4+2], bm.rgba[i*4+3]];

    // RGBA (32): stored R|G<<8|B<<16|A<<24, straight copy.
    let d = new Uint8Array(8); new DataView(d.buffer).setUint32(0,0xFF112233,true); new DataView(d.buffer).setUint32(4,0xFF445566,true);
    let bm = decodeZBitmap(makeRecord(0x52474241, 2, 1, [d], null), 0, 0x52474241);
    check("RGBA px0", JSON.stringify(px(bm,0)) === JSON.stringify([0x33,0x22,0x11,0xFF]));
    check("RGBA px1", JSON.stringify(px(bm,1)) === JSON.stringify([0x66,0x55,0x44,0xFF]));

    // PAL: palette u32 R-in-LSB.
    bm = decodeZBitmap(makeRecord(0x50414C4E, 2, 1, [new Uint8Array([0,1])], palBytes([0xFF000000, 0xFF0000FF])), 0, 0x50414C4E);
    check("PAL px0 black", JSON.stringify(px(bm,0)) === JSON.stringify([0,0,0,255]));
    check("PAL px1 red", JSON.stringify(px(bm,1)) === JSON.stringify([255,0,0,255]));

    // PAL_OPAC: alpha from the opacity buffer.
    let po = makeRecord(0x50414C4F, 1, 1, [new Uint8Array([1])], palBytes([0xFF000000, 0xFF332211]));
    // append 1-byte opacity buffer (size 1x1)
    let poFull = new Uint8Array(po.length + 1); poFull.set(po); poFull[po.length] = 0x80;
    bm = decodeZBitmap(poFull, 0, 0x50414C4F);
    check("PALOPAC rgb from pal, alpha from opac", JSON.stringify(px(bm,0)) === JSON.stringify([0x11,0x22,0x33,0x80]));

    // I8: v replicated into RGBA.
    bm = decodeZBitmap(makeRecord(0x49382020, 1, 1, [new Uint8Array([0x80])], null), 0, 0x49382020);
    check("I8 gray", JSON.stringify(px(bm,0)) === JSON.stringify([128,128,128,128]));

    // U8V8: R=255, G=U, B=V, A=255.
    let u = new Uint8Array(2); new DataView(u.buffer).setUint16(0, 0x4020, true); // U=0x20 V=0x40
    bm = decodeZBitmap(makeRecord(0x55385638, 1, 1, [u], null), 0, 0x55385638);
    check("U8V8", JSON.stringify(px(bm,0)) === JSON.stringify([255,0x20,0x40,255]));

    // DXT1: gtest block {0xF800,0x07E0,0x00E4,0x0000}: px0 red, px1 green, px3 (4-color).
    let b1 = new Uint8Array(8); { const v = new DataView(b1.buffer);
        v.setUint16(0,0xF800,true); v.setUint16(2,0x07E0,true); v.setUint32(4,0x000000E4,true); }
    bm = decodeZBitmap(makeRecord(0x44585431, 4, 4, [b1], null), 0, 0x44585431);
    check("DXT1 px0 red", JSON.stringify(px(bm,0)) === JSON.stringify([0xF8,0,0,255]));
    check("DXT1 px1 green", JSON.stringify(px(bm,1)) === JSON.stringify([0,0xFC,0,255]));

    // DXT1 3-color transparent index (c0<=c1): gtest {0,0xFFFF,0x00E4,0} → px3 alpha 0.
    let b2 = new Uint8Array(8); { const v = new DataView(b2.buffer);
        v.setUint16(0,0x0000,true); v.setUint16(2,0xFFFF,true); v.setUint32(4,0x000000E4,true); }
    bm = decodeZBitmap(makeRecord(0x44585431, 4, 4, [b2], null), 0, 0x44585431);
    check("DXT1 px0 black", JSON.stringify(px(bm,0)) === JSON.stringify([0,0,0,255]));
    check("DXT1 px3 transparent", px(bm,3)[3] === 0);

    // DXT3: alpha nibbles; all 0xF -> alpha 255, px0 red from c0.
    let b3 = new Uint8Array(16); { const v = new DataView(b3.buffer);
        v.setUint32(0,0xFFFFFFFF,true); v.setUint32(4,0xFFFFFFFF,true);
        v.setUint16(8,0xF800,true); v.setUint16(10,0x07E0,true); v.setUint32(12,0x000000E4,true); }
    bm = decodeZBitmap(makeRecord(0x44585433, 4, 4, [b3], null), 0, 0x44585433);
    check("DXT3 px0 red opaque", JSON.stringify(px(bm,0)) === JSON.stringify([0xF8,0,0,255]));

    // alphaBounds: only the visible pixels are bounded (gizmo tightly fits text).
    // 4x4 image with a single opaque pixel at (2,1).
    let rgba4 = new Uint8ClampedArray(4 * 4 * 4);
    rgba4[(1 * 4 + 2) * 4 + 3] = 255;
    const vb = alphaBounds({ width: 4, height: 4, rgba: rgba4 });
    check("alphaBounds single pixel", vb && vb.x === 2 && vb.y === 1 && vb.w === 1 && vb.h === 1);
    // Fully transparent -> null.
    check("alphaBounds empty -> null", alphaBounds({ width: 2, height: 2, rgba: new Uint8ClampedArray(2 * 2 * 4) }) === null);
}

// ---- real hideout file ----
(async () => {
    const buf = fs.readFileSync(process.argv[2]);
    const ab = buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
    const seq = await loadLoaderSequence(ab);
    check("real 4 pictures", seq.info.nrPictures === 4);
    check("real 4 keyframes", seq.info.nrKeyFrames === 4);
    check("real screen 1024x768", seq.screen.x === 1024 && seq.screen.y === 768);
    check("real fullprogress 6", approx(seq.fullProgressTime, 6));
    const names = seq.pictures.map(p => p.name).sort().join(",");
    check("real names", names === "00_Background,01_Kill,Red_Bar,WhiteBar");
    // Each picture has 4 sprites
    const spriteCounts = seq.pictures.map(p => p.sprites.length);
    check("real sprites 4 each", spriteCounts.every(c => c === 4));
    // Sprites tile the full 1024x768 design area (no X shift, no overlap bands).
    for (const p of seq.pictures) {
        let minx = Infinity, miny = Infinity, maxx = -Infinity, maxy = -Infinity;
        for (const s of p.sprites) {
            minx = Math.min(minx, s.posX); miny = Math.min(miny, s.posY);
            maxx = Math.max(maxx, s.posX + s.texW); maxy = Math.max(maxy, s.posY + s.texH);
        }
        const ok = approx(minx, 0, 1) && approx(miny, 0, 1) && approx(maxx, 1024, 1) && approx(maxy, 768, 1);
        check("real " + p.name + " tiles 1024x768 (" + [minx,miny,maxx,maxy].map(v=>v.toFixed(0)) + ")", ok);
    }
    // Gizmo: the text overlay's visible bbox is far smaller than the screen;
    // the opaque splash layers fill it.
    function visArea(name) {
        const p = seq.pictures.find(x => x.name === name);
        let minx=1e9,miny=1e9,maxx=-1,maxy=-1;
        for (const s of p.sprites) { if (!s.vis) continue;
            const bx=s.posX+s.vis.x, by=s.posY+s.vis.y;
            minx=Math.min(minx,bx); miny=Math.min(miny,by);
            maxx=Math.max(maxx,bx+s.vis.w); maxy=Math.max(maxy,by+s.vis.h); }
        return (maxx-minx)*(maxy-miny);
    }
    check("gizmo: text overlay tight (<20% screen)", visArea("01_Kill") < 1024*768*0.20);
    check("gizmo: opaque layer ~full (>90% screen)", visArea("WhiteBar") > 1024*768*0.90);
    // Red_Bar X should depend on progress (Progress interp)
    const redIdx = seq.pictures.findIndex(p => p.name === "Red_Bar");
    seq.script.setProgress(0);
    const x0 = seq.script.getPosX(redIdx, 0);
    seq.script.setProgress(1);
    const x1 = seq.script.getPosX(redIdx, 0);
    console.log("Red_Bar posX progress0=" + x0.toFixed(2) + " progress1=" + x1.toFixed(2));
    check("real Red_Bar progress-dependent posX", !approx(x0, x1));
    check("real Red_Bar posX at progress0 == -922", approx(x0, -922, 0.5));
    console.log(fails === 0 ? "\nALL PASS" : ("\n" + fails + " FAILURES"));
    process.exit(fails === 0 ? 0 : 1);
})();
