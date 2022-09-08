@echo off

set Name=meGame

if not exist build mkdir build
pushd build

rem NO_COMPILE_COMANDS_HELPER

set Includes=/I..\Library\include /Ic:\physx\includes
rem /IE:\Autodesk\FBX\FBX_SDK\2020.2.1\include
set LibDir=/LIBPATH:"..\Library\lib" /LIBPATH:"c:\physx\debug"
rem /LIBPATH:"E:\Autodesk\FBX\FBX_SDK\2020.2.1\lib\vs2019\x64\release"

set OS_Libs=user32.lib gdi32.lib kernel32.lib opengl32.lib glfw3_mt.lib shell32.lib assimp-vc142-mt.lib glew32s.lib ^
PhysXExtensions_static_64.lib PhysXPvdSDK_static_64.lib PhysXVehicle_static_64.lib PhysXCharacterKinematic_static_64.lib ^
PhysX_64.lib PhysXCooking_64.lib PhysXCommon_64.lib PhysXFoundation_64.lib ^
imgui*.obj glad.obj

rem wininet.lib libfbxsdk.lib
set Definitions=/D_USE_MATH_DEFINES
rem /DGLEW_STATIC
set MoreDefs=
rem /DNDEBUG
rem /D3D_GAME_SLOW=1 /D3D_GAME_INTERNAL=1 /D3D_GAME_WIN32=1 /D__AVX__=1 /D__AVX2__=1 

set CommonCompilerFlags=%MoreDefs% %Definitions% /nologo /FC /wd4130 /wd4100 /wd4089 /wd4068 /wd4505 /wd4456 /wd4201 /wd4100 /wd4505 /wd4189 /wd4457 ^
/MTd /Oi /Od /GR- /Gm- /EHa- /EHsc /Zi  /std:c++17 /diagnostics:caret
rem /RTC1 

set CommonLinkerFlags=-incremental:no -opt:ref /NODEFAULTLIB:LIBCMT

set OtherSrcFiles=
rem ..\Library\include\glad\glad.c
rem ..\Library\include\imgui\imgui*.cpp

rem 32-bit build
rem cl %CommonCompilerFlags% 3d_asteroid-hero\code\main.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

rem 64-bit build
rem del *.pdb > NUL 2> NUL
REM Optimization switches: /O2 /Oi /fp:fast
cl %CommonCompilerFlags% %Includes% /Fe%Name%.exe ..\src\main.cpp %OtherSrcFiles% ^
/link %CommonLinkerFlags% %OS_Libs% %LibDir%

rem %Name%.exe
popd