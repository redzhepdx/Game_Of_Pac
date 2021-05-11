cmake_minimum_required(VERSION 3.10)

project(GameOfPack)

add_subdirectory(src)
add_subdirectory(libs/Segvcatch)

find_package(glfw3 3.2.1 REQUIRED)
find_package(Torch REQUIRED)

# find_package(OpenGL REQUIRED)

file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_link_directories(${PROJECT_NAME} PUBLIC src)

target_include_directories(${PROJECT_NAME} PUBLIC include include/Quadtree include/AI include/External/ libs/Segvcatch)

target_link_libraries(${PROJECT_NAME} ${TORCH_LIBRARIES} GL GLU glut SOIL bsd glfw boost_log boost_system boost_filesystem segvcatch cuda cublas cudnn)

file(COPY files DESTINATION ${CMAKE_BINARY_DIR})

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -Wconversion -Wsign-conversion -fcommon -fexceptions -fnon-call-exceptions -g -rdynamic")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
