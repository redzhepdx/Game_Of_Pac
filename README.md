# Game_Of_Pac
From Scratch OpenGL Game and ML Agent/Enemy 

# DONEs:
- Maze Generation with DFS 
- Shaking 
- Vibration Effect 
- Enemy Spawn 
- Pac-Man Style Maze Exit 
- Movement Smooting for the pass between two close tiles 
- Add Edge Connection(Mirror) 
- Random Position Generation
- Connection 
- Boundaries 
- Copy Function for the move unique_ptr objects 
- Rotate Bullets 
- Collision of Bullets 
- Reload 
- Memory based refactoring : unique_ptr lists
- A* Player Finding 
- Create Enemy Chase Action with A* Algorithm
- Bullet Collection 
- Enemy Health 
- Enemy Bullet Collection 
- Enemy Respawn 
- Cmake
- Libtorch

# IN-PROGRESS's
- AI
- Refactoring : Code Organization 
- Check QuadTree for Collision Detection(IF IT IS NECESSARY)

# TODOs:
- Jumping Areas 
- Screen Shaking
- Suicide Bomber

### Required Libs and Tools
```Cpp17```
```cmake 3.10.0 + ```
```Glfw3 3.2.1 +```
```SOIL```
```GL GLU glut```
```BSD```
```boost```
```torch 1.7.1(I use pip versioN)```

### Compile and Run
- Do it only once!

``` mkdir -p out/build ```

``` export Torch_DIR=/absolute/path/to/site-packages/torch/share/cmake/Torch ```

``` cd out/build/ ```

``` cmake -S ../../ -B .```

or 

```chmod +x configure.sh build.sh run.sh```

```./configure.sh```

- After every update!

``` make ``` or ```./build.sh```

- Start the game

``` ./GameOfPack``` or ```./run.sh```

### Controls

```W : Up```
```D : Down```
```A : Left```
```S : Right```

```Q : Anti Clock Wise Rotate```
```E : Clock Wise Rotate```

```X : Shoot```

```Esc : Quit```

```Contact``` -> ```redzhep12@gmail.com```

##### Enjoy!