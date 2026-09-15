<p align="center">
	<img width="400" height="400" src=".github/logo.png">
</p>

[![Build status](https://ci.appveyor.com/api/projects/status/300yrxlxkmeyi5le?svg=true)](https://ci.appveyor.com/project/DronCode/rehitman-9uw4c)

What is ReHitman?
-----------------

ReHitman is a project dedicated to reverse-engineering Hitman: Blood Money.

Project Goals:
 * Standalone Executable: Fully reverse-engineer the original binary to create a completely independent, buildable source base.
 * Game Scripts: Decompile and reverse-engineer all in-game scripts to regain their complete original source code.

Game Build Information.
------------------------

This project is built for the last build of Hitman Blood Money that IO Interactive released.

Executable String:

	blood-build3-20060616-26123

Install
-------

 1. Download the latest release
 2. Unpack `NVCPL.dll` & `ReHitman.dll` into the game folder
 3. Launch HitmanBloodMoney.exe

Current Project Goals
---------------------

 * [ ] Glacier 1 Engine
  * [x] ZSTL (`zstdlib`)
  * [x] Input system
  * [x] Animation system
    * [x] IK animation
    * [x] Weighted animation
    * [x] `.anm` file format
  * [ ] Audio subsystem
    * [x] DirectSound branch
    * [ ] OpenAL branch
    * [ ] DieselPower branch (unmaintained by IOI)
    * [x] AudioGraph
    * [x] Synth engine
  * [x] Filesystem
    * [x] Generic FS
    * [x] ZIP FS
  * [x] Action system
    * [x] Bindings (dynamic, static)
    * [x] Parsers
  * [x] Platform system (`ZSysInterface`)
  * [x] EventBase
    * [x] Event base core
    * [x] Script event base
  * [x] Geom system
    * [x] Base geom
    * [x] Geom extends
  * [x] COM system
  * [ ] UI
    * [ ] Legacy UI system (almost done)
    * [ ] XML UI system
  * [ ] Physics
    * [x] Fysix engine
    * [x] Rigid bodies
    * [x] Cloth simulation
    * [ ] `CRigidBody` event base
  * [x] Pathfinder4
  * [x] ScriptEngine
    * [x] Script engine core & `ZEventBase` integration
    * [x] Save&Load
  * [x] RTP (runtime properties)
  * [ ] Serialization layer (skipped some small classes, not required yet)
  * [ ] Network - skipped
  * [x] Material system - `BS_Runtime`
  * [ ] Render
    * [x] Camera system
    * [x] Frontend
    * [x] Backend (some fields not renamed)
    * [x] Objects & Instances
    * [ ] Materials
      * [x] Material binders
      * [ ] Render state (almost done)
    * [ ] Render context
    * [x] View system
    * [ ] Draw system
      * [x] Base interface
      * [x] Base impl
      * [ ] D3D9 - not finished `ZRenderDrawD3D::Update`
    * [x] Entries
    * [ ] Debug menus - WIP
  * [ ] Hitman: Blood Money
    * [ ] Gameplay
    * [ ] Actors
    * [ ] Player
    * [ ] Movement
    * [ ] LevelControls
    * [ ] Script bindings
    * [ ] Per level specific things

FAQ
-----
 * [Wiki.](https://github.com/ReGlacier/ReHitman/wiki)

Credits
------

 * [DronCode](https://github.com/DronCode)
 * [crabovwik](https://github.com/crabovwik)
 * [Daniel Hunter](https://github.com/HHCHunter)
 * [Notexe](https://github.com/Notexe)
 * [RealMiamo](https://github.com/RealMiamo)

Contact Information.
---------------

The team can be contacted in the Project's issue tracker or via Discord (any questions, ideas, etc).

<a href="https://discord.gg/V5grGRw">
	<img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" />
</a>
