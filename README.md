# Vulpine Game Engine

<p align="center">
<image width="400" height="400" src="https://github.com/MonsieurBleu/Game-Engine/assets/97893210/e051c85c-d509-4b99-ad3e-a5ffb777c521">
</image></p>

## Presentation

The Vulpine Engine is a lightweight game engine made from scratch in C++ using OpenGL. This project aims to provide a solid and versatile foundation for 3D game development. The engine is built for developers who, like me, prefer a hands-on, code first approach with both hight-level built-in features and low-level fine tuning through direct API calls.


## About The Creator 

My goals with this project, are to learn how to make an application such as this one and to have a custom-made graphics/game engine for sevral of my personal and academical projects. This engine is and will always be open source and free for all uses.

## Project Built With Vulpine 

- [Game Repo Example](https://github.com/MonsieurBleu/Vulpine-Engine-Game-Repo-Example) a simple game built with the engine to showcase how to setup things.
- ["Les bois de Vulpigniac"](https://github.com/MonsieurBleu/Renerds-Code-Game-Jam-2024) made for the Code Game Jam 2024.
- [Height Maps](https://github.com/MonsieurBleu/Height-Maps) first implemntation of terrain rendering.

## Portability & Dependencies 

The Vulpine Engine is supported for Windows and Linux environment with OpenGL 4.6 compatiblity. No Mac or Android/IOS support is planned or will ever be.

This application uses different libraries :
- [OpenGL](https://www.opengl.org/)
- [GLEW](https://github.com/nigels-com/glew)
- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm) 
- [KTX](https://github.com/KhronosGroup/KTX-Software)
- [STB](https://github.com/nothings/stb)
- [OpenAL Soft](https://github.com/kcat/openal-soft)
- [Mini-Vobris](https://github.com/edubart/minivorbis)

## Instalation & usage 

Important modules :
- [Vulpine Engine Shader Library](https://github.com/MonsieurBleu/VESL--Vulpine-Engine-Shader-Library)
- [Vulpine Engine Assets Convertor](https://github.com/MonsieurBleu/VEAC-Vulpine-Engine-Asset-Convertor)

Project structure with Vulpine :
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


## Features 

### Game Logic
- [X] Entity Conpoment System
- [ ] States Machine
- [ ] Script Utils
  
### Graphics 
- [X] Cameras
  - [X] Perspective Camera
  - [X] Ortographic Camera
  - [X] Easy To Save/Load Camera State
  - [X] Reversed Z 

- [X] Geometry Lighting
  - [X] Blinn-Phong
  - [X] PBR
  - [X] Multi-Light
  - [X] Ambient Light 
  - [X] Directionnal Light
  - [X] Point Light
  - [X] Tube Light

- [X] Colors & Buffer
  - [X] Easy Frame Buffer Creation & Usage
  - [X] HDR
  - [X] sRGB Support (OpenGL native) 

 - [ ] Shadowing
   - [X] Multi Shadow Maps Support
   - [X] Efficient Percentage Close Smooth Shadows
   - [X] Directionnal Light Shadow Mapping
   - [ ] Point Light Shadow Mapping 
   - [ ] Cascading Shadow Maps

- [ ] Geometry Drawning 
  - [X] Vao Drawning
  - [X] Automatic Uniform & Maps Update
  - [ ] Automatic UBO Updates 
  - [X] Single Instanced Vao Drawning
  - [ ] Multiple Instanced Vao Drawing
  - [ ] AZDO
    - [X] Bindless Textures   
    - [ ] Indirect Vao Drawning

- [X] Rendering Techniques
  - [X] Foward Rendering
  - [X] Clustured Rendering  

- [ ] Post Process
  - [X] SSAO
  - [X] Bloom
  - [X] ToneMapping
  - [X] Depth-Based Pixelisation (experimental)
  - [X] Chromatic Aberration
  - [ ] MSAA
  - [ ] FXAA
  - [ ] ... 

### Scene Management 
- [ ] Scene Hierarchy
  - [X] Automatic Updates
  - [X] Hide Status Automatic Propagation 
  - [X] For Object Groups
  - [X] For Meshes
  - [X] For ModelSates/Instances (TODO : test)
  - [X] For Point Lights
  - [ ] For Tube Lights
  - [ ] For Directionnal Lights
  - [ ] Fast Non-Redundant Updates
        
- [ ] Culling 
  - [X] Frustum
    - [X] Sphere Frustum Culling 
    - [ ] AABB Frustum Culling 
    - [ ] Fast Parallelized Frustum Culling
  - [ ] Efficient Occlusion Culling

- [ ] Map/Scene Editor 

### 2D Rendering
- [ ] Text
  - [X] MSDF Font Rendering
  - [X] UFT Support
  - [X] Single String Batch
  - [X] Markdown bold and italics
  - [ ] Multi String Batch
  - [ ] Automatic Text Resizing
      
- [x] Fast Ui for debugging
  - [X] Fast UI Batching 
  - [X] Value Menu Tab 
    - [X] Const & Input Support
    - [X] Int
    - [X] Float
    - [X] Bollean
    - [ ] String
    - [X] Color : Hue, Saturation, Value, HexCode
    - [X] Direction : Phi & Theta Angle
    - [X] Angles

### Inputs
- [X] Easy Text Inputs With Clipboard Support & Single Source Security
- [X] Inputs Handler
- [X] App's Default GLFW Inputs Callbacks
- [ ] Easy Custom Inputs Implementation
- [ ] Window Event Handler

### Utils & Debug
- [X] Clear OpenGl Message Callback
- [X] Lights/Physics/PathFinding/Frustum/Geometry Helpers
- [X] Angle & Color Conversion Functions
- [X] Precise Benchmark/Frame Timer
- [X] Precise Limit Timer
- [X] Default AppTime & SimulationTime Timers
- [X] Error/Warning Message Macros
- [X] Mapped Enum Maccros 

### Assets 
- [X] Asset Manager With Easy Text Based Constructor & Duplication Detection Optimization

- [X] Vulpine Engine Asset Convertor
  - [X] Assimp integration  
  - [X] KTX2 integration 
  - [X] .vulpineMesh
  - [X] .vulpineSkeleton
  - [X] .vulpineAnimation
  - [ ] .vulpineNavigationGraph
  - [ ] .vulpineBody
  - [ ] .vulpineLights
  - [ ] .vulpineScene

- [X] 3D
  - [X] Easy VAO Creation & Update
  - [X] Automatic AABB Model Generation   
  - [X] .obj Model Loading
  - [X] Custom Fast Model Format
      
 - [X] Textures
  - [X] png/jpeg/gif Images Loading (STB lib)
  - [X] hdr Images Loading (STB lib)
  - [X] KTX/KTX2 Images Loading (KTX lib) 
  - [X] MipMaps (KTX lib)
  - [X] Basic CubeMap Loading
  - [X] Font/CSV Atlas MSDF Loading
  - [ ] (optional) Level/Scene Atlas Generator

 - [X] Audio
   - [X] .ogg Files loading
   - [ ] .wav Files loading
   - [X] 3D Audio (OpenAL lib)
   - [ ] Audio Effects (Reverb, etc...)

- [ ] Shader
  - [X] Shader Include (With Custom Librairies) 
  - [X] Easy Fragment/Geometry/Vertex Shader Loading From Source File
  - [ ] Shared Device & Host Constants/Defines
  - [X] Easy Tesselation Shader Loading From Source File
  - [ ] PIR-V Loading
  - [ ] Compute Shader Loading 

- [ ] Assets Streaming
   - [X] Automatic & Secure Destroy Calls  
   - [ ] Audio Streaming
   - [ ] Scene/Models Automatic Streaming
   - [ ] Shader Streaming

### Physics
- [X] Base Physics Engine
- [X] Constant Step Physics Thread
- [X] Sphere Collider
- [X] AABB Collider
- [ ] OBB Collider
- [ ] Easy Colliders Loading
- [ ] Game Object Class
  - [X] Base Class
  - [ ] Multi Collider Support
  - [ ] Model/Physics Following mode
- [ ] Physics Culling/Clusering/Optimisation 

### AI 
- [X] Path Finding Nodes
- [X] Path Finding Debugger
- [X] A* Path Finding Algorithm
- [ ] Path Finding Nodes Generator From Level/World

