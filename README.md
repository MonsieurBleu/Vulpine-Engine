<h1 align="center"> Vulpine Engine </h1> <br>

<p align="center">
<image width="400" height="400" src="https://github.com/MonsieurBleu/Game-Engine/assets/97893210/e051c85c-d509-4b99-ad3e-a5ffb777c521">
</image></p>

<p align="center">
  Light, modular and smart as a fox!
</p>

## Presentation

Vulpine is a free and open-source game engine built in C++. It aims to provide powerful yet lightweight tools for creating games and interactive environments, featuring a unique take on hierarchical ECS and modding-oriented asset streaming.

## General Features

- 3D scene management
- Skeletal animation
- Clustered lighting
- Terrain rendering
- Input management and player controllers
- Built-in debug and benchmarking tools
- Custom geometry file format optimized for fast loading, compression, and decompression
- Custom JSON-like language designed to be easy to edit and highly efficient to parse
- Several image loading formats supported, including JPG, PNG, TIFF, GIF, KTX, EXR, and HDR
- 3D sound system powered by the OpenAL Soft project
- Asset conversion tools powered by the ASSIMP project
- Full Lua scripting support, including user-made bindings and multithreaded support, powered by the SOL2 project

## Unique features 

### Modular Entity Groupping

Game logic in Vulpine is handled through a unique Modular Entity Grouping system, designed to combine the modularity of ECS-based game logic with fully customizable object graph relationships and manipulation. In this module, each component can interact with the parent or children of its respective entity in different ways. These operations include syncing, reparenting, merging, merge compatibility checks, file writing/loading, and creation/deletion routines. This system can be used to create custom level manipulation, logic, and optimization that are both content-aware and logic-aware.

In the Sanctia project, for example, it is used to reduce draw calls and overall scene weight by merging all static elements while preserving the full integrity of the region's logic.


### Asset Management

All supported file formats that can be loaded by Vulpine use an Asset Manager, which scans the game files at each startup to compile loading information. Each asset is referenced only by its name, streamlining the creation process by removing the need to write or update specific file locations. If it’s there and you want it, the engine will find it for you.

Modding is also fully supported. Each asset inside a mod folder is marked by the engine and tracked separately. Mods can also be contained within other mods, creating sub-mods recursively without limitation. Overwriting or adding any game asset is as simple as dropping it into your mod folder. Any folder, anywhere, can be marked as a mod by adding [0-9] at the beginning of its name. The number inside the brackets corresponds to a default overwrite priority. For example, in the Sanctia project, 0 represents base game assets, 1 user mods, and 2 compatibility patches. This method streamlines the user experience by automatically handling the default mod order while still allowing the order of each mod to be modified separately in a dedicated file.

The last important point for modding support is that load order doesn’t exist in Vulpine, because the engine is aware of all available resources. This means mod order only affects the overwrite priority of assets, and no dependency errors can be caused by mod order.


## Planned Features

- Pathfinding
- Cascaded shadow maps
- Procedural texture generation tools
- Improved controller support
- Game implementation example using the tools and ECS created for the Sanctia project


## Project Built With Vulpine 

- [(deprecated) Game Repo Example](https://github.com/MonsieurBleu/Vulpine-Engine-Game-Repo-Example) a simple game built with the engine to showcase how to setup things.
- ["Les bois de Vulpigniac"](https://github.com/MonsieurBleu/Renerds-Code-Game-Jam-2024) made for the Code Game Jam 2024.
- [Height Maps](https://github.com/MonsieurBleu/Height-Maps) first implemntation of terrain rendering.
- [Real Time Spatially Varying Noise](https://github.com/MonsieurBleu/Real-Time-Spatially-Varying-Noise), a simple application to create and test 2D noises created for a master thesis.
- Sanctia, an open-world medieval role-simulation currently in development.

## Portability & Dependencies 

The Vulpine Engine is supported for Windows and Linux environment with OpenGL 4.6 compatiblity.

This application uses different libraries :
- [OpenGL](https://www.opengl.org/)
- [GLEW](https://github.com/nigels-com/glew)
- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [Assimp](https://github.com/assimp/assimp)
- [KTX](https://github.com/KhronosGroup/KTX-Software)
- [STB](https://github.com/nothings/stb)
- [OpenAL Soft](https://github.com/kcat/openal-soft)
- [Mini-Vobris](https://github.com/edubart/minivorbis)
- [Tiny EXR](https://github.com/syoyo/tinyexr)
- [Sol2](https://github.com/ThePhD/sol2)
- [LuaJIT](https://luajit.org/luajit.html)

## Instalation & usage 

Important modules :
- [Vulpine Engine Shader Library](https://github.com/MonsieurBleu/VESL--Vulpine-Engine-Shader-Library)
- [(deprecated, now included inside the engine) Vulpine Engine Assets Convertor](https://github.com/MonsieurBleu/VEAC-Vulpine-Engine-Asset-Convertor)

Project structure recommended with Vulpine :
```
my-project/
├── build/
│   ├── ressources/
│   ├── saves/
│   ├── shader/  <===== VESL submodule
│   └── Game.exe
├── Engine/      <===== Vulpine Engine submodule
├── include/
│   └── Game.hpp
├── obj/
│   └── Game.o
├── src/
│   └── Game.cpp
├── .gitattributes
├── .gitignore
├── .gitmodules
├── Makefile
└── main.cpp
```



