CC = g++
CPPFLAGS = -Wall -g
LIBFLAGS = -I include -L./ -lmingw32 -lglew32 -lglfw3 -lopengl32 
LINKFLAGS = libglfw3.a libglfw3dll.a 
OBJ = obj/main.o obj/App.o obj/Utils.o obj/Shader.o obj/Camera.o
INCLUDE = -Iinclude 
EXEC = GameEngine.exe
DEL_win = del /Q /F

# g++ -o hellot.exe main.cpp libglfw3.a libglfw3dll.a -I include -L./ -lglew32 -lglfw3 -lopengl32 

default: $(EXEC)

run :
	$(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) $(LINKFLAGS) -o $(EXEC) $(LIBFLAGS)

install : $(EXEC)

reinstall : clean install

obj/main.o : main.cpp
	$(CC) -c $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) $< -o $@ 

obj/%.o : src/%.cpp
	$(CC) -c $(CPPFLAGS) $(LIBFLAGS) $(INCLUDE) $< -o $@ 

PHONY : clean

clean : 
	$(DEL_win) $(EXEC) obj\*.o

countlines :
	find ./ -type f \( -iname \*.cpp -o -iname \*.hpp -o -iname \*.frag -o -iname \*.vert -o -iname \*.geom \) | sed 's/.*/"&"/' | xargs  wc -l