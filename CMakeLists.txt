cmake_minimum_required(VERSION 3.10)
project(TutorialOpenGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(assimp CONFIG REQUIRED)
find_package(glm REQUIRED)

# Lista de archivos corregida (sin prefijos de carpetas)
add_executable(MiPrograma3D 
    Main.cpp
    camara.cpp
    glad.c
    stb.cpp
    camara.h
)

target_link_libraries(MiPrograma3D PRIVATE assimp::assimp glm::glm)
