#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 CameraDir;
out vec3 LightDir;

out vec4 ClipSpace;
out vec2 TexCoords;

// Imports the camera matrix
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

uniform vec3 CameraPos;
uniform vec3 LightPos;

const float Tiling = 4.0;
void main()
{
    TexCoords = aTexCoords * Tiling;

    vec4 FragPos = Model * vec4(aPos, 1.0);
    ClipSpace = Projection * View * FragPos;

    CameraDir =  CameraPos - FragPos.xyz;
 	LightDir  =  LightPos - FragPos.xyz;

	gl_Position = ClipSpace;
}







// TexCoords = vec2( aPos.x/2.0 + 0.5, aPos.y/2.0 + 0.5);
