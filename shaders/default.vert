#version 430 core


layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;
layout(location = 5) in vec3 aTangent;
layout(location = 6) in vec3 aBiTangent;


out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 ViewDir;
out vec3 LightDir;

// out vec3 TangentLightPos;
// out vec3 TangentViewPos;
// out vec3 TangentFragPos;

out vec4 FragPosLightSpace;


uniform vec3 lightPos;
uniform vec3 viewPos;
uniform mat4 lightSpaceMatrix;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// const vec4 clip_plane = vec4(0,-1,0, -208);
// const vec4 clip_plane = vec4(0,1,0, 255);
uniform vec4 clip_plane;


void main()
{
	vec4 FragPosV4 = model *  vec4(aPos, 1.0f);


	FragPos = FragPosV4.xyz;
	Normal = aNormal;
	TexCoords =  aTex;
	

	gl_ClipDistance[0] = dot(FragPosV4, clip_plane);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = aBiTangent;
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vec3 TangentLightPos = TBN * lightPos;
    vec3 TangentViewPos  = TBN * viewPos;
    vec3 TangentFragPos  = TBN * FragPos;

    ViewDir = normalize(TangentViewPos - TangentFragPos);
    LightDir = normalize(TangentLightPos - TangentFragPos);

	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);	
	
	gl_Position = projection * view * vec4(FragPos, 1.0);
}