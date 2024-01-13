CC = g++
CPPFLAGS = -Wall -Ofast -Wno-strict-aliasing
ifeq ($(OS),Windows_NT)
	LIBFLAGS = -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32  -lktx -lOpenAL32
	LINKFLAGS = libglfw3.a libglfw3dll.a 
else
	LIBFLAGS = -L./ -lGLEW -lglfw -lGL -lktx -lopenal
	LINKFLAGS = 
endif

INCLUDE = -Iinclude -IexternalLibs 
ifeq ($(OS),Windows_NT)
	EXEC = GameEngine.exe
	RM = del /Q /F /S
else
	EXEC = GameEngine
	RM = rm -f
endif

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ODIR=obj
IDIR=include
SDIR=src

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
DEPFLAGSMAIN = -MT $@ -MMD -MP -MF $(DEPDIR)/main.d

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
	G_EXEC = ..\build\Game.exe
endif

default: $(EXEC)

game: $(G_EXEC)

gameClean : 
	$(RM) $(G_EXEC) ..\obj\*.o

gameReinstall : gameClean game

$(G_EXEC): $(G_OBJ) $(G_EOBJ)
	$(CC) $(G_EOBJ) $(G_OBJ) $(LINKFLAGS) -o $(G_EXEC) $(LIBFLAGS)

../obj/main.o : ../main.cpp
../obj/main.o : ../main.cpp 
	$(CC) -c $(CPPFLAGS) -I../include -Wdelete-non-virtual-dtor $(LIBFLAGS) $(INCLUDE) $< -o $@

../obj/%.o : ../src/%.cpp
../obj/%.o : ../src/%.cpp
	$(CC) -c $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) -I../include $< -o $@ 

#/**************************************/




run :
	$(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LINKFLAGS) -o $(EXEC) $(LIBFLAGS)

install : $(EXEC)

reinstall : clean install

obj/main.o : main.cpp
obj/main.o : main.cpp $(DEPDIR)/main.d | $(DEPDIR)
	$(CC) -c $(DEPFLAGSMAIN) $(CPPFLAGS) -Wdelete-non-virtual-dtor $(LIBFLAGS) $(INCLUDE) $< -o $@

obj/MINIVOBRIS_IMPLEMENTATION.o : src/MINIVOBRIS_IMPLEMENTATION.cpp
	$(CC) -c $(CPPFLAGS) -fpermissive -w $(LIBFLAGS) $(INCLUDE)  $< -o $@

obj/%.o : src/%.cpp
obj/%.o : src/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(CC) -c $(DEPFLAGS) $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) $< -o $@ 



$(DEPDIR): ; @mkdir $@

DEPFILES := $(SOURCES:$(SDIR)/%.cpp=$(DEPDIR)/%.d)
DEPFILES += $(DEPDIR)/main.d
# $(info $(DEPFILES))
$(DEPFILES):


include $(wildcard $(DEPFILES))



clean : 
	$(RM) $(EXEC) obj\*.o $(DEPDIR)\*.d

countlines :
	find ./ -type f \( -iname \*.cpp -o -iname \*.hpp -o -iname \*.frag -o -iname \*.vert -o -iname \*.geom \) | sed 's/.*/"&"/' | xargs  wc -l