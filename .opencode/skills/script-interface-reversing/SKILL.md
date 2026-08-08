---
name: script-interface-reversing
description: Reverse Glacier ScriptInterfaces entries in bounded PC/iOS/PS2 batches, resolve collisions, rename meaningful PC functions, add signature-correct TODO stubs, and persist resumable progress until the complete table is registered and named.
compatibility: opencode with PC, iOS, and PS2 reverse-engineering tools available
metadata:
  project: ReHitman
  workflow: iterative-reversing
---
# ScriptInterfaces Reversing

## Objective

Reconstruct the PC `ScriptInterfaces` table in the project by correlating its entries with `_ScriptInterfaces` on iOS and `ScriptInterfaces` on PS2. Register every accepted method in table order, add a declaration and minimal implementation with the correct signature, and rename each meaningful corresponding function in the PC reverse-engineering database.

The implementation target is `ReHitman/ScriptInterface`. The primary table initializer is `source/SI.cpp`; its layout declaration is `include/SI/SI.h`.

## Durable State

Use these files as the source of truth across sessions:

- `source/SI.cpp`: completed entries and the exact next iOS/PS2 table checkpoint.
- `include/SI/SI.h`: completed typed table fields and remaining reserved slots.
- `PROGRESS.md`: table cursor, independent PC naming backfill cursor, current checkpoint, unresolved decisions, latest verification, and status.

At all times, `SI.cpp` must contain exactly one next-method comment while work remains:

```cpp
// Next method iOS: {mangled name} {function address} Off: {table entry address} | PS2: {mangled name} {function address} Off: {table entry address}
```

Replace this line after every completed batch. Do not append a history of old checkpoints. When no entries remain, replace it with:

```cpp
// ScriptInterfaces reversing complete: 0x2CC entries
```

## Batch Loop

Repeat this loop without waiting for user confirmation:

1. Recover the next index and iOS/PS2 entry addresses from `SI.cpp` and cross-check `PROGRESS.md`.
2. Select 4-8 consecutive entries, defaulting to 6.
3. Read each iOS and PS2 table pointer and identify/decompile its target. Locate the corresponding PC function by symbols, references, constants, call structure, and behavior.
4. Verify table alignment before editing. If an entry exists on only one platform, investigate neighboring entries rather than silently shifting the table.
5. Resolve the exported name using the collision rules below.
6. Infer the exact return type and parameter list from the clearest reversed platform. Confirm ABI-sensitive types against PC call sites where possible.
7. Rename each meaningful corresponding PC function according to the accepted method name, following the PC Naming rules. Classify empty stubs without renaming them.
8. Add declarations and TODO implementations to logically grouped files, add typed fields to `SI.h`, and add indexed pointers to `SI.cpp`.
9. Update CMake and includes only when the batch introduces a new logical source/header pair.
10. Move the next-method checkpoint to the first unprocessed entry and update `PROGRESS.md` atomically with the code and PC naming results.
11. Verify indices, table size/layout, declarations/definitions, PC names/classifications, and compilation. Fix failures before starting another batch.
12. Clear transient analysis from working context and continue with the next batch.

## Name Collisions

Names differ between iOS and PS2 when their demangled identities are not equivalent after accounting for ABI mangling and parameter-width differences.

Apply these rules in order:

1. If a candidate method or feature mentions `Ferral` (case-insensitive), reject it because that feature is absent from the PC build. Confirm table alignment around the rejected platform-only entry and record the decision in `PROGRESS.md`; do not create a PC stub for it.
2. If PC behavior matches iOS more closely, use the iOS method name.
3. If PC behavior matches PS2 more closely, use the PS2 method name.
4. If both match, prefer the name that best describes the shared behavior and is consistent with nearby existing PC names; record why.
5. If neither can be established from pseudocode, compare assembly, callers, callees, constants, RTTI, and data accesses.
6. If the choice is still ambiguous, checkpoint the evidence and ask the user one focused question. Do not guess.

Compare logical behavior, not compiler artifacts such as register allocation, calling convention boilerplate, pointer width, or equivalent mangling differences.

## PC Naming

Rename the corresponding PC database function as soon as an entry's identity is established. Use the same accepted logical name used by the project declaration, preserving the PC database's established naming convention when it requires a mangled or qualified representation.

- Verify the PC address by behavior and table/call-site evidence before renaming.
- Use compare-and-set protection when the rename tool supports it: supply the observed current name and do not force-overwrite a concurrently changed name.
- If the PC function already has the accepted meaningful name, mark it verified without changing it.
- Do not rename a function merely to mirror an iOS or PS2 mangled spelling; name the PC function for the accepted PC method identity.
- Record the PC address and result as `renamed`, `already named`, or `empty stub` in the current batch data until the progress cursor is advanced.

Never rename dummy placeholders or semantically empty functions. This includes names or bodies such as `jpeg_mem_term`, `nullstub_*`, `dummy`, no-op return stubs, and equivalent functions that perform no meaningful engine behavior. Confirm emptiness from the body rather than relying only on a suspicious current name. For such entries:

1. Leave the original PC function name unchanged.
2. If the reverse-engineering tool supports function comments, set a concise comment such as `ScriptInterfaces[245]: Zactor__Usereservations; intentionally not renamed because this PC function is an empty stub.`
3. Mark the entry `empty stub` in PC naming coverage so it does not block completion.

If a function is a non-empty forwarding thunk, it is not an empty stub: rename it when it represents the script entry. If emptiness or the correct PC target is uncertain, inspect assembly and callers before deciding.

## Existing-Entry Naming Backfill

PC naming is tracked independently from table reconstruction so previously registered entries can be named without redoing project stubs.

Use `PC Naming Coverage` in `PROGRESS.md` as a monotonically increasing backfill cursor. Process already registered entries in consecutive batches of 4-8:

1. Read the accepted method name and index from `SI.cpp` and its signature from `SI.h`/the owner header.
2. Correlate the iOS/PS2 implementation with the PC build and identify the verified PC function address. Do not assume the project function pointer is the original PC address.
3. Apply the PC Naming rules: rename a meaningful function, verify an existing good name, or classify/comment an empty stub.
4. Advance the backfill cursor only after every index in the batch has one of those three terminal outcomes.
5. Store only the latest backfill batch summary and the next backfill index in `PROGRESS.md`; do not accumulate all addresses in chat or a permanent transcript.

The dedicated `/backfill-script-interface-pc-names` command runs this mode for all currently registered entries. Normal forward reversing must also name every new entry immediately. If forward reversing reaches completion before backfill, continue backfill until PC naming coverage reaches the final applicable index.

## Signatures And Stubs

- Signatures must have the correct return type and arguments at this stage.
- Prefer the platform whose method was successfully reversed and validate against PC usage.
- For PS2 return types, use the xexe search tool with the mangled symbol only; ordinary names are not supported.
- Preserve project types and conventions. Do not invent engine layouts or stubs for unreversed classes.
- Group methods by logical owner, for example `ZCar` in `SI_ZCar.h/.cpp` and `Zhm3Actor__Die` in `SI_ZHM3Actor.h/.cpp`.
- Follow existing capitalization and namespace style in neighboring ScriptInterface files.
- Every new implementation body starts with exactly `// TODO: Finish me`.
- Return nothing for `void`, `false` for `bool`, `0` for integral and pointer-like zero results, `0.0f` for `float`, `0.0` for `double`, and `{}` for aggregates. Use the project's established null convention when it differs.
- Do not implement real behavior during this workflow.

For each registered method, add the typed field to `ScriptInterfacesTable` and a pointer in `ScriptInterfacesDefault` with its decimal index:

```cpp
&Zactor__Usereservations, // 245
```

Reduce or remove the trailing `Reserved_*` field so `static_assert(sizeof(SI::ScriptInterfacesTable) == sizeof(void*) * 0x2CC)` remains valid. Never reorder completed entries.

## Progress File

Keep `ReHitman/ScriptInterface/PROGRESS.md` short and machine-resumable with these headings:

- `Status`: `IN_PROGRESS`, `BLOCKED`, or `COMPLETE`.
- `Coverage`: total table entries, completed registered entries, next index, and last completed index.
- `PC Naming Coverage`: reviewed count, last reviewed index, next backfill index, and terminal result counts for renamed, already named, and empty stubs.
- `Checkpoint`: the same next iOS/PS2 data as `SI.cpp`, or `none` when complete.
- `Latest Batch`: only the most recently completed index range and verification result.
- `Latest PC Naming Batch`: only the most recently reviewed PC naming range and result counts.
- `Open Decisions`: unresolved collisions/blockers with addresses and concise evidence, or `none`.

Update the existing values; do not grow an audit log. A rejected platform-only `Ferral` entry is not a registered PC entry, so explain any resulting alignment adjustment under `Open Decisions` until it is fully verified.

## Verification

After every batch:

- Confirm new `SI.cpp` comments are consecutive and match the intended indices.
- Confirm `SI.h` field order exactly matches `SI.cpp` pointer order.
- Confirm every pointer has one compatible declaration and definition.
- Confirm the next iOS and PS2 offsets point to the first unprocessed aligned entries.
- Confirm every newly registered entry has a verified PC address and a terminal PC naming result.
- For a naming backfill batch, confirm every index was renamed, already correctly named, or verified as an empty stub; confirm empty stubs were not renamed.
- Confirm the table remains exactly `0x2CC` pointer slots.
- Run the narrowest available ScriptInterface build. If the repository provides no usable configured build, perform static consistency checks and record that limitation.
- Never hide a failing verification. Fix it or set `Status: BLOCKED` with the exact failure.

## Completion Criteria

Set `Status: COMPLETE` and stop only when all conditions hold:

1. All `0x2CC` PC table slots, indices `0..715`, have been accounted for in order.
2. Every applicable PC entry has an accepted name, a signature-correct declaration, a typed `SI.h` field, an indexed `SI.cpp` pointer, and a minimal TODO implementation.
3. Every meaningful corresponding PC database function has the accepted name, including all entries that predated this workflow.
4. Every dummy or semantically empty PC function was deliberately left unrenamed, was commented when supported, and is classified as `empty stub` in completed naming coverage.
5. PC naming coverage accounts for every applicable final table index with no unresolved naming target.
6. Every omitted iOS/PS2-only `Ferral` entry has documented alignment evidence and does not leave an unexplained PC slot.
7. `ScriptInterfacesTable` has no remaining reserved slots and its size assertion passes.
8. There is no `// Next method ...` comment; `SI.cpp` contains the completion comment.
9. `PROGRESS.md` has `next index: none`, `next PC naming backfill index: none`, `Checkpoint: none`, and no unresolved decisions.
10. The ScriptInterface target builds successfully, or all available project verification passes and any environment-only build limitation is explicitly recorded.

A completed batch is a checkpoint, not task completion.
