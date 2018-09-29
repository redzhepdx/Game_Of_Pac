LIBS = -lglut -lGLU -lGL -DGL_GLEXT_PROTOTYPES -lSOIL -lbsd
CPPS = main.cpp GameWindow.cpp Sprite.cpp Vector2.cpp PlayerSprite.cpp Area.cpp Maze.cpp
CONF = `pkg-config --static --libs glfw3`
FirstGameTest: main.cpp
	g++ -std=c++14 -o firstGame $(CPPS) $(CONF) $(LIBS)
