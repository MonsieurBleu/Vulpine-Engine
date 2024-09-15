#************** COMPILER SETUP VARIABLES **************#
CC = clang++
WFLAGS = -Wall -Wno-strict-aliasing 
WFLAGS += -Wno-delete-non-abstract-non-virtual-dtor  
WFLAGS += -Wno-unused-private-field
WFLAGS += -Wno-unused-lambda-capture 
WFLAGS += -Wno-delete-non-virtual-dtor

OPTFLAGS = -Ofast

CPPFLAGS = $(WFLAGS) --std=c++20 $(OPTFLAGS)
INCLUDE = -Iinclude -IexternalLibs 


ifeq ($(OS),Windows_NT)
	LIBFLAGS = -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32 -lktx -lsoft_oal 
	LINKFLAGS = libglfw3.a libglfw3dll.a 
	VULPINE_LIB_NAME = TODO
else
	LIBFLAGS = -L./ -lGLEW -lglfw -lGL -lktx -lopenal -lX11 
	LINKFLAGS = 
	VULPINE_LIB_NAME = libvulpineEngine.so
endif

VULPINE_LIB_PATH = VULPINE_LIB_NAME



#************** HELPER VARIABLE/FUNCTIONS **************#
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
ECHO = echo
BOLD = [1m
UNDERLINE = [4m
INVERSE = [7m
RESET = [0m
GREEN = [32m
RED = [31m
BLUE = [34m
CYAN = [36m
ORANGE = [38;5;208m

ifeq ($(OS),Windows_NT)
	STRDEC = 
else 
	STRDEC = "
endif

BUILD_VULPINE = echo $(STRDEC) $(BOLD)$(UNDERLINE)$(ORANGE)-VULPINE LIBRARY BUILT-$(RESET) $(STRDEC)
BUILD_FILE_VULPINE = echo $(STRDEC) $(BOLD)$(ORANGE)Building Vulpine Module$(RESET) $(STRDEC)
BUILD_FILE_GAME    = echo $(STRDEC) $(BOLD)$(CYAN)Building$(RESET) $(STRDEC)
LINKING_EXECUTABLE = echo $(STRDEC) $(BOLD)$(UNDERLINE)$(BLUE)Linking$(RESET)$(STRDEC)

DEPFLAGS_BASE = -MT $@ -MMD -MP -MF .deps
VDEPFLAGS = $(DEPFLAGS_BASE)/$*.d
GDEPFLAGS = $(DEPFLAGS_BASE)/game/$*.d
GDEPFLAGS_MAIN= $(DEPFLAGS_BASE)/game/main.d


#************** DEFINING SOURCE FILES **************#
VSOURCES = $(call rwildcard,src,*.cpp)
VOBJ = $(VSOURCES:src/%.cpp=obj/%.o)
DEPFILES := $(VSOURCES:src/%.cpp=.deps/%.d)


GSOURCES = $(call rwildcard,../src,*.cpp)
GOBJ = $(GSOURCES:../src/%.cpp=obj/game/%.o) obj/game/main.o
DEPFILES += $(GSOURCES:../src/%.cpp=.deps/game/%.d) .deps/game/main.d

$(DEPFILES):

include $(wildcard $(DEPFILES))



#************** VULPINE SHARED LIBRARY BUILDING **************#
vulpine : $(VOBJ)
	@$(CC) -shared $(VOBJ) $(LINKFLAGS) -o $(VULPINE_LIB_PATH) 
	@$(BUILD_VULPINE)

obj/MINIVOBRIS_IMPLEMENTATION.o : src/MINIVOBRIS_IMPLEMENTATION.cpp
	@clang -c -x c -Wno-error -Ofast -fPIC $< -o $@
	@$(BUILD_FILE_VULPINE) $<

obj/%.o : src/%.cpp
	@$(CC) -c $(VDEPFLAGS) $(CPPFLAGS) -fPIC $(INCLUDE) $< -o $@ 
	@$(BUILD_FILE_VULPINE) $<



#************** GAME BUILDING **************#
ifeq ($(OS),Windows_NT)
GEXEC = ..\build\Game.exe
else
GEXEC = ../build/Game
endif

game : VULPINE_LIB_PATH = ../build/$(VULPINE_LIB_NAME) 
game : vulpine
game : LIBFLAGS += -lreactphysics3d -lvulpineEngine 
game : LINKFLAGS += -L../build -Wl,-rpath,'$$ORIGIN'
game : $(GOBJ)
	@$(LINKING_EXECUTABLE) $@
	@$(CC) $(GOBJ) $(LINKFLAGS) $(LIBFLAGS) -o $(GEXEC)

obj/game/main.o : ../main.cpp
	@$(CC) -c $(GDEPFLAGS_MAIN) $(CPPFLAGS) -I../include -Wno-delete-non-virtual-dtor $(INCLUDE) $< -o $@
	@$(BUILD_FILE_GAME) $<

obj/game/%.o : ../src/%.cpp
	@$(CC) -c $(GDEPFLAGS) $(CPPFLAGS) $(INCLUDE) -I../include $< -o $@ 
	@$(BUILD_FILE_GAME) $<



#************** SECONDARY COMMANDS **************#
clean : 
ifeq ($(OS),Windows_NT)
	@del /s /f /q $(EXEC) obj\*.o .deps\*.d 
else
	@rm -f $(EXEC) obj/*.o obj/game/*.o .deps/*.d .deps/game/*.d
endif

