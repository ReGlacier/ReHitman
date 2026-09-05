# Glacier COM System

This directory contains Glacier's COM key-value storage system. It is used for
small typed runtime variables: configuration values, global engine state, script
or mission parameters, and other name-addressed data.

The system has four main parts:

1. `CSharedCom` defines the common typed read/write API.
2. `CCom` stores values in a fixed 16 KB local arena.
3. `CGlobalCom` stores values in a caller-provided global buffer.
4. `CComRead` is a lightweight read proxy returned by `Get()` and `operator[]`.

## Basic Usage

`CCom` is the simplest standalone COM container:

```cpp
#include <Glacier/Com/CCom.h>

Glacier::CCom com;

com.SetVal("iHealth", 100);
com.SetVal("fSpeed", 1.5f);
com.SetVal("bAlive", true);
com.SetVal("sName", "Agent47");

int health = com["iHealth"];
float speed = com.Get("fSpeed");
bool alive = com["bAlive"];
const char* name = com.GetVal("sName");
```

Names are not just names. The default `SetVal()` overloads use the name prefix
to pick a COM type from `CCOMTypeTable`. In normal use, keep the conventional
prefixes stable:

| Prefix | Typical value | Format |
| --- | --- | --- |
| `b` | Boolean flags | `CCOM_FORMAT_BOOL` |
| `i` | 32-bit integers and references | `CCOM_FORMAT_int32` |
| `f` | Floats | `CCOM_FORMAT_FLOAT` |
| `s` | Strings and files | `CCOM_FORMAT_CHAR` |

The exact prefix mapping is engine data, stored in `CCOMTypeTable`. If a name's
prefix does not match the value format, `CSharedCom::AddVal()` asserts.

## Common Interface

`CSharedCom` is the abstract base class shared by all COM stores. It provides
typed overloads for simple values, arrays, strings, `REFTAB`, raw blocks, and
explicit `CCOMType` insertion.

```cpp
void WriteGameplayValues(Glacier::CSharedCom& com)
{
    com.SetVal("iAmmo", 30);
    com.SetVal("fAccuracy", 0.85f);
    com.SetVal("bSuspicious", false);
    com.SetVal("sOutfit", "guard");

    const int route[] = { 1, 4, 9 };
    com.SetVal("iRoute", route, 3);
}
```

Reads can either copy data into a caller-owned variable or return a pointer to
the stored payload:

```cpp
int ammo = 0;
if (com.GetVal("iAmmo", &ammo) != 0)
{
    // ammo was copied from the COM value.
}

const char* outfit = com.GetVal("sOutfit");
int routeBytes = com.GetDataLen("iRoute");
```

`GetVal(char* outBuffer, const char* name, int nameLen)` copies the raw payload
and returns the number of bytes copied. `GetValPtr()` returns a pointer into the
COM-owned storage, so that pointer becomes invalid when the value is removed,
overwritten, cleared, or the owning COM is destroyed.

## CCom

`CCom` is a local fixed-capacity implementation. It owns a 16 KB inline memory
arena and indexes live values with a `REFTAB32`.

Each stored value has this logical layout:

```text
+----------------+--------------+---------------+------------+
| Alloc header   | ComValueInfo | Name (NUL)    | Data       |
| 4 bytes        | 12 bytes     | lNameLen bytes| lDataLen   |
+----------------+--------------+---------------+------------+
```

The allocation header belongs to `CCom`; `CSharedCom` methods work with the
`ComValueInfo` pointer. Memory is managed by `ZOffsetAlloc`, so removing a value
immediately returns its block to the local free list.

```cpp
Glacier::CCom com;

com.SetVal("iScore", 5000);
com.SetVal("fTime", 12.5f);

if (com.Exists("iScore", 0))
{
    int score = com["iScore"];
}

com.RemoveVal("fTime", 0);
com.Clear();
```

`CopyAll()` deep-copies entries from another `CCom` into this instance:

```cpp
Glacier::CCom source;
Glacier::CCom copy;

source.SetVal("iAmmo", 30);
copy.CopyAll(&source);

int ammo = copy["iAmmo"]; // 30
```

`CCom` is useful for short-lived or object-local data where the fixed 16 KB
capacity and 32-entry index are sufficient.

## CGlobalCom

`CGlobalCom` implements the same `CSharedCom` API over a caller-supplied memory
buffer. It is intended for process-wide COM state and has global engine
instances exposed as `g_pGlobalCOM`, `g_GlobalCom`, and the generic
`g_pGlobalCom` pointer.

```cpp
#include <Glacier/Com/CGlobalCom.h>

char buffer[4096];
Glacier::CGlobalCom com(buffer, sizeof(buffer));

com.SetVal("iLevel", 5);
com.SetVal("sMission", "Flatline");
```

Each entry is appended to the buffer with an extra `CGlobalComMemInfo` header:

```text
+-------------------+--------------+---------------+------------+
| CGlobalComMemInfo | ComValueInfo | Name (NUL)    | Data       |
| size + used       | 12 bytes     | lNameLen bytes| lDataLen   |
+-------------------+--------------+---------------+------------+
```

Removal is lazy: `RemoveVal()` marks an entry as unused. Space is reclaimed when
`Defrag()` compacts live entries, or automatically when appending would exceed
the buffer capacity.

```cpp
com.SetVal("iOld", 1);
com.RemoveVal("iOld", 0);
com.Defrag();
```

When a new value has the same name and the same total entry size as an existing
one, `Append()` overwrites the existing entry in place. If the size changes, the
old entry is marked unused and the new entry is appended.

`CGlobalCom` can enumerate live keys:

```cpp
char key[256];

for (unsigned index = 0; com.EnumKeys(key, sizeof(key), index); ++index)
{
    printf("%s\n", key);
}
```

## CComRead

`CComRead` is a small read-only proxy. It stores a `CSharedCom*` and a key name,
then performs the actual lookup when converted to a concrete type.

```cpp
Glacier::CComRead value = com.Get("iHealth");

int health = value;
uint32_t flags = com["iFlags"];
float speed = com["fSpeed"];
bool enabled = com["bEnabled"];
char* title = com["sTitle"];
```

The conversions call `GetVal()` or `GetValPtr()` internally. Missing numeric and
boolean values convert to zero or `false`; missing strings convert to `nullptr`.

Because `CComRead` stores the key pointer, pass stable names such as string
literals or keep the name storage alive until the conversion happens.

## Type Metadata

`ComValueInfo` is the per-value metadata used by both storage implementations:

```cpp
struct ComValueInfo
{
    int lNameLen;
    CCOMTypeCast* lType;
    int lDataLen;
};
```

The bytes immediately after `ComValueInfo` are the key name, followed by the raw
payload. The type pointer refers to a `CCOMTypeCast` entry:

```cpp
struct CCOMTypeCast
{
    int lLetters;       // also addressable as four prefix bytes
    int lTypeLen;       // number of prefix bytes to match
    CCOMType eComType;  // semantic COM type
    int eLen;           // fixed payload size, or 0 for variable-size types
    int eCComFormat;    // accepted CCOMFormat bit mask
};
```

`CCOMType` describes the semantic type stored in COM, such as
`CCOM_TYPE_BOOL`, `CCOM_TYPE_INT32`, `CCOM_TYPE_FLOAT`, `CCOM_TYPE_STRING`,
`CCOM_TYPE_FILE`, `CCOM_TYPE_DATA`, or engine-specific reference types.
`CCOMFormat` describes the physical value format accepted by a `SetVal()`
overload.

Use explicit type overloads when the semantic type cannot be inferred from the
name alone:

```cpp
com.SetVal("sConfigPath", "system.ini", Glacier::CCOM_TYPE_FILE);
com.SetVal("iEntityRef", 0x1234, Glacier::CCOM_TYPE_REF);
```

## Header Reference

| Header | Responsibility |
| --- | --- |
| `CSharedCom.h` | Common abstract interface and typed `SetVal()` / `GetVal()` helpers |
| `CCom.h` | Fixed 16 KB local COM implementation backed by `REFTAB32` and `ZOffsetAlloc` |
| `CGlobalCom.h` | Buffer-backed global COM implementation with lazy deletion and defragmentation |
| `CComRead.h` | Read proxy used by `Get()` and `operator[]` |
| `ComValueInfo.h` | Per-entry name length, type pointer, and payload length |
| `CCOMType.h` | Semantic COM type IDs |
| `CCOMFormat.h` | Physical value format flags used for overload validation |
| `CCOMTypeCast.h` | Prefix-to-type mapping records stored in `CCOMTypeTable` |
| `Globals.h` | Global `CSharedCom*` access point |

## Important Rules

- Keep value names alive for APIs that store or defer name pointers, especially
  `CComRead`.
- Include the NUL terminator when passing an explicit name length. Passing `0`
  lets the implementation use `strlen(name) + 1`.
- Do not keep pointers returned by `GetVal()` or `GetValPtr()` after mutation of
  the owning COM.
- Match name prefixes, `CCOMFormat`, and `CCOMType`; invalid combinations assert.
- Use explicit `CCOMType` overloads for semantic types such as files,
  references, blocks, and engine-specific handles.
- Treat `CCom` capacity as fixed: it has a 16 KB arena and a `REFTAB32` live
  entry index.
- Call `Defrag()` on `CGlobalCom` if many values were removed and predictable
  free space is needed before more appends.
- Do not change field order or virtual method order in these classes. The sizes
  are verified for binary compatibility with the original Glacier engine.
