<h1 align="center"> Vulpine Engine </h1> <br>

<p align="center">
<image width="400" height="400" src="https://github.com/MonsieurBleu/Game-Engine/assets/97893210/e051c85c-d509-4b99-ad3e-a5ffb777c521">
</image></p>

<p align="center">
  Light, modular and smart as a fox!
</p>

## Presentation

Vulpine is a free and open source game engine built in C++. It aims at giving powerfull yet lightweight tools to make games and interactible environement, such as an unique take on hierarchical ECS and modding oriented asset streaming.

## General Features

- 3D scene management
- Skeletal animations
- Clustered Lighting
- Terrain rendering
- Input management and player controllers
- Built-in debug and benchmark tools
- Custom fast to load, compress and decompress geometry file format
- Custom JSON-like language made to be easy to edit and very efficient to parse
- Servral image loading supports includes jpg, png, tiff, gif, ktx, exr, hdr
- 3D Sound system, powered by the OpenAL Soft project
- Asset conversion tools, powered by the ASSIMP project
- Full LUA scripting support including user-made bindings and multithread support, powered by the SOL2 project

## Unique features 

### Modular Entity Groupping

Game logic in Vulpine is done with a unique Modular Entity Grouping system, made to combine the modularity of ECS based game logic with fully customazible object graph relation and manipulation. In this module, each component can have a different way of interacting with parent or children of their respective entity, these operation includes synching, reparenting, merging, merge compatibility checks, file writing/loading and creation/deletion routines. This system can be used to create custom level manipulation, logic and optimization that are content and logic aware.

In the Sanctia project, for instance, it is used to reduce drawcalls and general scene weight by merging all static elements while retaining the full integrity of the region's logic.


### Asset Management

All supported files format that can be loaded by Vulpine uses an Asset Manager that scans the game files at each startup to compile loading informations. Each asset is referenced only by his name, streamlining the creation process by removing any obligations to write or update specific file locations. If it's in there, and you want it, the engine will find it for you.

Modding is also fully supported, each asset inside a mod folder will be marked by the engine and will be tracked seperatly. Mods can also be contained inside other mods, creating a sub-mods, this can be done recursivly without any limitations. Overwriting or adding any game asset can be simply done by dropping it inside your mod folder. It is that easy ! Any folder, anywhere, can be marked as a mod by simply adding ``[0-9]`` at the beginning of his name. The number inside the bracket correspond to a default overwriting importance. For exemple, in the Sanctia project, 0 represents base game assets, 1 user mods and 2 compatibility patchs. This metod streamline the user experience by handling automaticly default mod order, while still giving the option to modify the order of each mod seperatly in a dedicated file.

The last important point for modding support is that load order doesn't exist in Vulpine, because the engine is aware of all available ressources. This leads the mod order to simply affects overwrite important of assets, but no dependencies error can be triggered by the mod order.


## Planned Features

- Path finding
- Cascaded shadowmap
- Procedural texture generation tools
- Better controller supports
- Game implementation example using tools and ECS made for the Sanctia Project


## Project Built With Vulpine 

- [Game Repo Example](https://github.com/MonsieurBleu/Vulpine-Engine-Game-Repo-Example) a simple game built with the engine to showcase how to setup things.
- ["Les bois de Vulpigniac"](https://github.com/MonsieurBleu/Renerds-Code-Game-Jam-2024) made for the Code Game Jam 2024.
- [Height Maps](https://github.com/MonsieurBleu/Height-Maps) first implemntation of terrain rendering.

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
- [(deprecated) Vulpine Engine Assets Convertor](https://github.com/MonsieurBleu/VEAC-Vulpine-Engine-Asset-Convertor)

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



