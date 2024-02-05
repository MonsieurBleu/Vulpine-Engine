CC = g++
CPPFLAGS = -Wall -Ofast -Wno-strict-aliasing -g --std=c++23
ifeq ($(OS),Windows_NT)
	LIBFLAGS = -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32  -lktx -lsoft_oal
	LINKFLAGS = libglfw3.a libglfw3dll.a 

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

	BUILD_FILE_VULPINE = $(ECHO) $(BOLD)$(ORANGE)$(UNDERLINE)Building Vulpine Module$(RESET)
	BUILD_FILE_GAME    = $(ECHO) $(BOLD)$(CYAN)$(UNDERLINE)Building$(RESET)
	LINKING_EXECUTABLE = $(ECHO) $(UNDERLINE)$(BOLD)$(BLUE)Linking$(RESET)

else
	LIBFLAGS = -L./ -lGLEW -lglfw -lGL -lktx -lopenal
	LINKFLAGS = 

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

	BUILD_FILE_VULPINE = $(ECHO) "$(BOLD)$(ORANGE)$(UNDERLINE)Building Vulpine Module$(RESET)"
	BUILD_FILE_GAME    = $(ECHO) "$(BOLD)$(CYAN)$(UNDERLINE)Building$(RESET)"
	LINKING_EXECUTABLE = $(ECHO) "$(UNDERLINE)$(BOLD)$(BLUE)Linking$(RESET)"

endif

INCLUDE = -Iinclude -IexternalLibs 
ifeq ($(OS),Windows_NT)
	EXEC = GameEngine.exe
	RM = del /s /f /q
else
	EXEC = GameEngine
	RM = rm -f
endif

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ODIR=obj
IDIR=include
SDIR=src

DEPDIR := .deps
DEPFLAGS_BASE = -MT $@ -MMD -MP -MF $(DEPDIR)
DEPFLAGS = $(DEPFLAGS_BASE)/$*.d
DEPFLAGSMAIN = $(DEPFLAGS_BASE)/main.d

SOURCES := $(call rwildcard,$(SDIR),*.cpp)
OBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)
OBJ += $(ODIR)/main.o

default: $(EXEC)

#/******* GAME BUILD ARGUMENTS ********/

G_ODIR=../obj
G_SDIR=../src

G_SOURCES := $(call rwildcard,$(G_SDIR),*.cpp)
G_OBJ := $(G_SOURCES:$(G_SDIR)/%.cpp=$(G_ODIR)/%.o)
G_EOBJ := $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)
G_EOBJ += $(G_ODIR)/main.o

ifeq ($(OS),Windows_NT)
	G_EXEC = ..\build\Game.exe
else
	G_EXEC = ../build/Game
endif

default: $(EXEC)

game: $(G_EXEC)

gameClean : 
ifeq ($(OS),Windows_NT)
	$(RM) $(G_EXEC) ..\obj\*.o
else
	$(RM) $(G_EXEC) ../obj/*.o
endif

gameReinstall : gameClean game

$(G_EXEC): $(G_OBJ) $(G_EOBJ)
	@$(LINKING_EXECUTABLE) $@
	@$(CC) $(G_EOBJ) $(G_OBJ) $(LINKFLAGS) -o $(G_EXEC) $(LIBFLAGS)

../obj/main.o : ../main.cpp
../obj/main.o : ../main.cpp 
	@$(CC) -c $(CPPFLAGS) -I../include -Wno-delete-non-virtual-dtor $(LIBFLAGS) $(INCLUDE) $< -o $@
	@$(BUILD_FILE_GAME) $<

../obj/%.o : ../src/%.cpp
../obj/%.o : ../src/%.cpp
	@$(CC) -c $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) -I../include $< -o $@ 
	@$(BUILD_FILE_GAME) $<

#/**************************************/

run :
	$(EXEC)

$(EXEC): $(OBJ)
	@$(LINKING_EXECUTABLE) $@
	@$(CC) $(OBJ) -o $@ $(LIBFLAGS) $(LINKFLAGS)

install : $(EXEC)

reinstall : clean install

obj/main.o : main.cpp
obj/main.o : main.cpp $(DEPDIR)/main.d | $(DEPDIR)
	@$(CC) -c $(DEPFLAGSMAIN) $(CPPFLAGS) -Wno-delete-non-virtual-dtor $(LIBFLAGS) $(INCLUDE) $< -o $@
	@$(BUILD_FILE_VULPINE) $<

obj/MINIVOBRIS_IMPLEMENTATION.o : src/MINIVOBRIS_IMPLEMENTATION.cpp
	@$(CC) -c $(CPPFLAGS) -fpermissive -w $(LIBFLAGS) $(INCLUDE)  $< -o $@
	@$(BUILD_FILE_VULPINE) $<

obj/Audio.o : src/Audio.cpp
	@$(CC) -c $(DEPFLAGS_BASE)/Audio.d $(CPPFLAGS) $(LIBFLAGS) -Wno-unused-variable $(INCLUDE) $< -o $@ 
	@$(BUILD_FILE_VULPINE) $<

obj/%.o : src/%.cpp
obj/%.o : src/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	@$(CC) -c $(DEPFLAGS) $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) $< -o $@ 
	@$(BUILD_FILE_VULPINE) $<

$(DEPDIR): ; @mkdir $@

DEPFILES := $(SOURCES:$(SDIR)/%.cpp=$(DEPDIR)/%.d)
DEPFILES += $(DEPDIR)/main.d
# $(info $(DEPFILES))
$(DEPFILES):


include $(wildcard $(DEPFILES))




clean : 
ifeq ($(OS),Windows_NT)
	@$(RM) $(EXEC) obj\*.o $(DEPDIR)\*.d 
else
	@$(RM) $(EXEC) obj/*.o $(DEPDIR)/*.d
endif

countlines :
	find ./ -type f \( -iname \*.cpp -o -iname \*.hpp -o -iname \*.frag -o -iname \*.vert -o -iname \*.geom \) | sed 's/.*/"&"/' | xargs  wc -l