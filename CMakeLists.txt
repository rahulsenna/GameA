cmake_minimum_required(VERSION 3.18)
project(meGame)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(/D_USE_MATH_DEFINES /DGLEW_STATIC)
add_compile_options(
        -Wno-string-compare
        # /MTd
        #        /Ox
        #        /diagnostics:caret
)

add_link_options(
        # /NODEFAULTLIB:LIBCMT
)

include_directories(
        ${PROJECT_SOURCE_DIR}/Library/include
        ~/dev/lib/PhysX/physx/include
        ~/dev/lib/PhysX/pxshared/include
        ~/dev/lib/PhysX/physx/source/foundation/include
        ~/dev/lib/assimp/include
        /usr/local/include

)
link_directories(
        ${PROJECT_SOURCE_DIR}/Library/lib
        ~/dev/lib/PhysX/physx/bin/linux.clang/release
        ~/dev/lib/assimp/bin

        /usr/lib
        /usr/local/lib
        /usr/local/lib64

)

file(GLOB ImGuiSrc Library/include/imgui/imgui*.cpp)
file(GLOB SRC src/*.cpp)


add_executable(meGame
               ${SRC}
               ${ImGuiSrc}
               #               Library/include/glad/glad.c
               )


target_link_libraries(meGame
        X11
        GL

        libglfw3.a
        libGLEW.so
        libassimp.so
        libphysxrepack.a
)