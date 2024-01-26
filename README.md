# Vulpine Game Engine

<p align="center">
<image width="400" height="400" src="https://github.com/MonsieurBleu/Game-Engine/assets/97893210/e051c85c-d509-4b99-ad3e-a5ffb777c521">
</image></p>

## Features 

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
  - [X] Single Instanced Vao Drawning
  - [X] Multiple Instanced Vao Drawing
  - [ ] Indirect/AZDO Vao Drawning

- [ ] Rendering Techniques
  - [X] Foward Rendering
  - [ ] Deffered Rendering
  - [ ] Clustured Rendering  

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
  - [ ] For Point/Tube Lights
  - [ ] For Directionnal Lights
  - [ ] Fast Non-Redundant Updates
        
- [ ] Culling 
  - [ ] Frustum
    - [X] Sphere Frustum Culling 
    - [ ] AABB Frustum Culling 
    - [ ] Fast Parallelized Frustum Culling
  - [ ] Efficient Occlusion Culling

### 2D 
- [ ] Text
  - [X] MSDF Font Rendering
  - [X] Single String Batch
  - [ ] Multi String Batch
  - [ ] Multi Text Style Per Batch Support
  - [ ] Automatic Text Resizing
      
- [ ] Fast Ui
  - [X] Fast UI Batching
  - [X] Value Menu Tab (TODO : Refactor this monstrosity)
    - [X] Const & Input Support
    - [ ] Output Support
    - [X] Int
    - [X] Float
    - [X] Bollean
    - [ ] String
    - [X] Color : Hue, Saturation, Value, HexCode
    - [X] Direction : Phi & Theta Angle
    - [X] Angles

### Inputs
- [ ]

### Assets 
- [ ] 3D
  - [X] Easy VAO Creation & Update
  - [X] Automatic AABB Model Generation   
  - [X] .obj Model Loading
  - [ ] Custom Fast Model Format

 - [ ] Textures
  - [X] png/jpeg/gif Images Loading (STB lib)
  - [X] hdr Images Loading (STB lib)
  - [X] Ktx Images Loading (KTX lib) 
  - [X] MipMaps (KTX lib)
  - [X] Basic CubeMap Loading
  - [X] Font/CSV Atlas MSDF Loading
  - [ ] Level/Scene Atlas Generator

 - [ ] Audio
   - [X] .ogg Files loading
   - [ ] .wav Files loading
   - [X] 3D Audio (OpenAL lib)
   - [ ] Audio Effects (Reverb, etc...)

- [ ] Shader
  - [X] Shader Include (With Custom Librairies) 
  - [X] Easy Fragment/Geometry/Vertex Shader Loading From Source File
  - [ ] Shared Device & Host Constants/Defines
  - [ ] Easy Tesselation Shader Loading From Source File
  - [ ] Easy SPIR-V Loading
  - [ ] Easy Compute Shader Loading 

- [ ] Assets Streaming 
   - [ ] Audio Streaming
   - [ ] Scene/Models Automatic Streaming
   - [ ] Shader Streaming


  

