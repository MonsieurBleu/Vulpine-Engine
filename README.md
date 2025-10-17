

<p align="center">
<image width="400" height="400" src="https://github.com/MonsieurBleu/Game-Engine/assets/97893210/e051c85c-d509-4b99-ad3e-a5ffb777c521">
</image></p>


<p align="center">
  Light, modular and smart as a fox!
</p>

<h1 align="center"> Vulpine Engine </h1> <br>

<p align="center">
Vulpine is a free and open-source game engine built in C++. It aims to provide powerful yet lightweight tools for creating games and interactive environments, featuring a unique take on hierarchical ECS and modding-oriented asset streaming.
</p>

<p align="center">
  Follow us on &nbsp; <a href="https://bsky.app/profile/vulpinegames.bsky.social">
    <img src="https://upload.wikimedia.org/wikipedia/commons/7/7a/Bluesky_Logo.svg" width="20">
    Blue Sky
  </a> &nbsp; &nbsp; <a href="https://www.youtube.com/@VulpineGameStudio">
    <img src="https://upload.wikimedia.org/wikipedia/commons/e/ef/Youtube_logo.png" width="20">
    Youtube
  </a> 

</p>

<p align="center">

  
</p>

<p align="center">


  
</p>



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

Game logic in Vulpine is handled through a unique Modular Entity Grouping module, designed to combine the modularity of ECS-based game logic with fully customizable object graph relationships and manipulation. This system can be used to create content-aware level manipulation, logic and optimization.

### Asset Management

> A wise fox never loses what it values.

Vulpine uses a custom Asset Manager that references all assets by their name, ensuring that no paths are required for anything from shader includes to textures. If it's in there, Vulpine will find it!
This system also keeps track of all loaded assets and avoids loading the same asset multiple times. 
Modding support is also fully implemented, giving the ability to create or add anything easily and manage the importance order of every mod. Everything that Vulpine loads can be modded at will.

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
├── Game/
│   ├── data/
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
├── Makefile
└── main.cpp
```



