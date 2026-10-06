---
description: Rename verified PC functions for already registered ScriptInterfaces entries
agent: script-interface-reverser
subtask: true
---
Run as an isolated subtask. Load the `script-interface-reversing` skill and run Existing-Entry Naming Backfill from the PC naming cursor in `ReHitman/ScriptInterface/PROGRESS.md` through every currently registered ScriptInterfaces index. Work in bounded consecutive batches, persist the cursor after each batch, rename only verified meaningful PC functions, and leave verified dummy or empty stubs unrenamed with address comments when supported. Continue autonomously until naming coverage catches up to the current table coverage or a fully investigated ambiguous PC target requires one focused user decision. Do not add or re-reverse table entries in this command. Return a concise summary only after the durable checkpoint has been written.
