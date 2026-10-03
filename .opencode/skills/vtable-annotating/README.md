# Vtable Annotating

This skill reconstructs a C++ vtable from a class header and compiler ABI, then adds a separate IDA comment to every verified vtable slot through Hyper MCP.

## Usage

Provide the header path, exact class name, vtable address, Hyper/IDA instance, and compiler ABI:

```text
/annotate-vtable header ReHitman/Glacier/include/ZGROUP.h, class ZGROUP, vtable address 0x0076A4C4, instance PC, ABI MSVC
```

A natural-language request works as well:

```text
/annotate-vtable Annotate the vtable for class ZGROUP declared in ReHitman/Glacier/include/ZGROUP.h. The vtable starts at 0x0076A4C4 in the PC instance and uses the MSVC ABI.
```

The compiler ABI may be omitted initially:

```text
/annotate-vtable ReHitman/Glacier/include/ZGROUP.h, class ZGROUP, vtable 0x0076A4C4, instance PC
```

In that case, the agent will ask whether the target uses the MSVC or Clang ABI before analyzing the table.

## Expected Result

For a 32-bit vtable, comments are added to consecutive four-byte slots:

```text
0076A4C4  ~ZSerializable
0076A4C8  ZSerializable::PreSave
0076A4CC  ZSerializable::PostLoad
...
```

Comments are attached to the vtable slot addresses, not to the function addresses stored in those slots. The skill includes inherited methods and compiler-generated destructor or thunk slots when confirmed by the selected ABI and compiler vtable dump.

## Files

- Workflow instructions: `.opencode/skills/vtable-annotating/SKILL.md`
- Slash command: `.opencode/commands/annotate-vtable.md`
