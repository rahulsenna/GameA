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
out vec2 texCoord;

out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

out vec4 FragPosLightSpace;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform mat4 lightSpaceMatrix;


const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesTransformations[MAX_BONES];

uniform vec4 clip_plane;


void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(aPos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesTransformations[boneIds[i]] * vec4(aPos,1.0f);
        totalPosition += localPosition * weights[i];
        // vec3 localNormal = mat3(finalBonesTransformations[boneIds[i]]) * aNormal;
   }
    vec4 crntPosV4 = vec4(model * totalPosition);
    // vec4 crntPosV4 = vec4(model * vec4(aPos,1.0f));
    
    gl_ClipDistance[0] = dot(crntPosV4, clip_plane);

    
    FragPos = crntPosV4.xyz;
    Normal = transpose(inverse(mat3(model))) * aNormal;
	texCoord = aTex;



    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = aBiTangent;
    
    mat3 TBN = transpose(mat3(T, B, N));    
    TangentLightPos = TBN * lightPos;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;


    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = projection * view * crntPosV4;

}