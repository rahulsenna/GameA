cmake_minimum_required(VERSION 3.22)
project(meGame)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(/D_USE_MATH_DEFINES /DGLEW_STATIC)
add_compile_options(
        /MTd
        #        /Ox
        #        /diagnostics:caret

)

add_link_options(
        /NODEFAULTLIB:LIBCMT
)

include_directories(
        ${PROJECT_SOURCE_DIR}/Library/include
        c:/physx/includes
)
link_directories(
        ${PROJECT_SOURCE_DIR}/Library/lib
        c:/physx/debug
)

file(GLOB ImGuiSrc Library/include/imgui/imgui*.cpp)
file(GLOB SRC src/*.cpp)


add_executable(meGame
               ${SRC}
               ${ImGuiSrc}
               #               Library/include/glad/glad.c
               )


target_link_libraries(meGame
                      opengl32.lib
                      glfw3_mt.lib
                      glew32s.lib

                      shell32.lib
                      assimp-vc142-mt.lib

                      PhysX_64.lib
                      PhysXFoundation_64.lib
                      PhysXCommon_64.lib
                      PhysXCooking_64.lib

                      PhysXExtensions_static_64.lib
                      PhysXPvdSDK_static_64.lib
                      PhysXVehicle_static_64.lib
                      PhysXCharacterKinematic_static_64.lib

                      )