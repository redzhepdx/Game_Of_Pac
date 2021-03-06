cmake_minimum_required(VERSION 3.10)

project(GameOfPack)

add_subdirectory(src)

find_package(glfw3 3.2.1 REQUIRED)
find_package(Torch REQUIRED)
# find_package(OpenGL REQUIRED)

file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_link_directories(${PROJECT_NAME} PUBLIC src)

target_include_directories(${PROJECT_NAME} PUBLIC include include/quadtree)

target_link_libraries(${PROJECT_NAME} ${TORCH_LIBRARIES} GL GLU glut SOIL bsd glfw)

file(COPY files DESTINATION ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 --no-pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)