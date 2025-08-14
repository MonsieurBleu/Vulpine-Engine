#************** COMPILER SETUP VARIABLES **************#
# CC = /bin/time -f "\n\ttime : %e \n" clang++
CC = clang++
WFLAGS = -Wall -Wno-strict-aliasing 
WFLAGS += -Wno-delete-non-abstract-non-virtual-dtor  
WFLAGS += -Wno-unused-private-field
WFLAGS += -Wno-unused-lambda-capture 
WFLAGS += -Wno-delete-non-virtual-dtor
WFLAGS += -Wno-unused-variable

OPTFLAGS =

CPPFLAGS = $(WFLAGS) --std=c++20 $(OPTFLAGS)
INCLUDE = -Iinclude -IexternalLibs -I../externalLibs -I /usr/include/lua5.4/


#  TODO : update links for windows

ifeq ($(OS),Windows_NT)
	LIBFLAGS = -L./ -Llibs/ -lmingw32 -lglew32 -lglfw3 -lopengl32 -lktx -lsoft_oal -lpthread -lgvc -lcgraph -lcdt
	LINKFLAGS = libglfw3.a libglfw3dll.a
	VULPINE_LIB_NAME = libvulpineEngine.a
else
	LIBFLAGS = -L./ -Llibs/ -lGLEW -lglfw -lGL -lktx -lopenal -lX11 -lgvc -lcgraph -lcdt -lluajit-5.1
	LINKFLAGS = 
	VULPINE_LIB_NAME = libvulpineEngine.a
endif


ifeq ($(OS),Windows_NT)
	VULPINE_LIB_PATH = ..\build\$(VULPINE_LIB_NAME)
else
	VULPINE_LIB_PATH = ../build/$(VULPINE_LIB_NAME)
endif



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
# turns out printing in windows is really annoying, also the user could be using powershell or cmd
# which have different ways of printing
	ECHO_N = echo  
else 
	STRDEC = "
	ECHO_N = echo -n
endif

ESC=
CLEAR_LINE=${ESC}[2K
MOVE_UP=${ESC}[1A
MOVE_DOWN=${ESC}[1B

CR=


TO_LIB_LINE = $(ECHO_N) ${STRDEC} ${MOVE_UP} ${CLEAR_LINE} \r ${STRDEC}
TO_PRO_LINE = $(ECHO_N) ${STRDEC} ${CLEAR_LINE} \r ${STRDEC}

# TO_PRO_LINE = bash -c 'lines=$$(tput lines); echo -e  "\033[2K\033[G"' 
# TO_LIB_LINE = bash -c 'lines=$$(tput lines); echo -e "\033[1A;1H\033[2K\033[G"'

BUILD_VULPINE      = ${TO_LIB_LINE} && echo ${STRDEC}${BOLD}${UNDERLINE}${ORANGE}-VULPINE LIBRARY BUILT-${RESET}${STRDEC}
BUILD_FILE_VULPINE = ${TO_LIB_LINE} && echo ${STRDEC}${BOLD}${ORANGE}Vulpine Module Built ${RESET}${STRDEC}
BUILD_FILE_GAME    = ${TO_PRO_LINE} && $(ECHO_N) ${STRDEC}${BOLD}${CYAN}Built ${RESET}${STRDEC} 
LINKING_EXECUTABLE = ${TO_PRO_LINE} && $(ECHO_N) ${STRDEC}${BOLD}${UNDERLINE}${BLUE}Linking${RESET}${STRDEC} 

BUILD_GAME    = ${TO_PRO_LINE} && $(ECHO_N) ${STRDEC}${BOLD}${CYAN}Builting Game...${RESET}${STRDEC} 
GAME_READY = ${TO_PRO_LINE} && echo ${STRDEC}${BOLD}${UNDERLINE}${BLUE}Game is now compiled and linked!${RESET}${STRDEC} 

DEPFLAGS_BASE = -MT $@ -MMD -MP -MF .deps
VDEPFLAGS = $(DEPFLAGS_BASE)/$*.d
GDEPFLAGS = $(DEPFLAGS_BASE)/game/$*.d
GDEPFLAGS_MAIN= $(DEPFLAGS_BASE)/game/main.d


#************** DEFINING SOURCE FILES **************#
VSOURCES = $(call rwildcard,src,*.cpp)
VOBJ = $(VSOURCES:src/%.cpp=obj/%.o)
DEPFILES := $(VSOURCES:src/%.cpp=.deps/%.d)


GSOURCES = $(call rwildcard,../src,*.cpp)
GOBJ = $(GSOURCES:../src/%.cpp=obj/game/%.o) obj/game/main.o $(VULPINE_LIB_PATH)
DEPFILES += $(GSOURCES:../src/%.cpp=.deps/game/%.d) .deps/game/main.d

$(DEPFILES):

include $(wildcard $(DEPFILES))



#************** VULPINE SHARED LIBRARY BUILDING **************#
vulpine : $(VOBJ)
# @$(CC) -shared $(VOBJ) $(LINKFLAGS) $(LIBFLAGS) -o $(VULPINE_LIB_PATH) 
	@ar rcs $(VULPINE_LIB_PATH) $(VOBJ)
	@$(BUILD_VULPINE)

$(VULPINE_LIB_PATH) : vulpine

obj/MINIVOBRIS_IMPLEMENTATION.o : src/MINIVOBRIS_IMPLEMENTATION.cpp
	@clang -c -x c -Wno-error $(OPTFLAGS) $(INCLUDE) -fPIC $< -o $@
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

game_base_echo : 
	@echo " "
	@${BUILD_GAME} 


game : vulpine
game : LIBFLAGSG = $(LIBFLAGS) -lreactphysics3d
#game : LINKFLAGS += -L../build -Wl,-rpath,'$$ORIGIN'
game : game_base_echo $(GOBJ)
	@$(LINKING_EXECUTABLE) $@
	@$(CC) $(GOBJ) $(LINKFLAGS) $(LIBFLAGSG) -o $(GEXEC)
	@${GAME_READY}

obj/game/main.o : ../main.cpp
	@$(CC) -c $(GDEPFLAGS_MAIN) $(CPPFLAGS) -I../include -Wno-delete-non-virtual-dtor $(INCLUDE) $< -o $@
	@$(BUILD_FILE_GAME) $<

obj/game/%.o : ../src/%.cpp
	@$(CC) -c $(GDEPFLAGS) $(CPPFLAGS) $(INCLUDE) -I../include $< -o $@ 
	@$(BUILD_FILE_GAME) $<



#************** SECONDARY COMMANDS **************#
clean : 
ifeq ($(OS),Windows_NT)
	@del /s /f /q $(EXEC) obj\*.o .deps\*.d obj\game\*.o .deps\game\*.d $(VULPINE_LIB_PATH)
else
	@rm -f $(EXEC) obj/*.o obj/game/*.o .deps/*.d .deps/game/*.d $(VULPINE_LIB_PATH)
endif

