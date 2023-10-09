CC = g++
CPPFLAGS = -Wall -g -Wno-strict-aliasing
LIBFLAGS = -I include -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32  -lktx
LINKFLAGS = libglfw3.a libglfw3dll.a 
# IMG = obj/main.o obj/App.o obj/Utils.o obj/Shader.o obj/Camera.o obj/Uniforms.o obj/VertexBuffer.o obj/Mesh.o obj/Scene.o obj/Inputs.o obj/FrameBuffer.o obj/Textures.o obj/RenderPass.o obj/Timer.o obj/Globals.o
# IMG = $(wildcard obj/*.cpp)
INCLUDE = -Iinclude 
EXEC = GameEngine.exe
DEL_win = del /Q /F


ODIR=ktx
SDIR=png

SOURCES := $(wildcard $(SDIR)/*.png)
IMG := $(SOURCES:$(SDIR)/%.png=$(ODIR)/%.ktx)

# g++ -o hellot.exe main.cpp libglfw3.a libglfw3dll.a -I include -L./ -lglew32 -lglfw3 -lopengl32 


install : $(IMG)

reinstall : clean install

ktx/%.ktx : png/%.png
	toktx $@ $<

