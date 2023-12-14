CC = g++
CPPFLAGS = -Wall -Ofast -Wno-strict-aliasing -g
ifeq ($(OS),Windows_NT)
	LIBFLAGS = -I include -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32  -lktx
	LINKFLAGS = libglfw3.a libglfw3dll.a 
else
	LIBFLAGS = -I include -L./ -lGLEW -lglfw -lGL -lktx
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

run :
	$(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LINKFLAGS) -o $(EXEC) $(LIBFLAGS)

install : $(EXEC)

reinstall : clean install

obj/main.o : main.cpp
obj/main.o : main.cpp $(DEPDIR)/main.d | $(DEPDIR)
	$(CC) -c $(DEPFLAGSMAIN) $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) $< -o $@

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