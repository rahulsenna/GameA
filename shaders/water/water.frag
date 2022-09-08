#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 ClipSpace;

in vec3 CameraDir;
in vec3 LightDir;

uniform sampler2D ReflectionTex;
uniform sampler2D RefractionTex;
uniform sampler2D DUDV_Map;
uniform sampler2D NormalMap;
uniform sampler2D DepthMap;

uniform vec4 LightColor;
uniform float MoveFactor;


const float WaveStrength = 0.04;

const float Shininess = 20.0;
const float SpecularStrength = 0.3;

void main(void) {

	vec2 NDC = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;



	vec2 RefractionCoors = NDC;
	vec2 ReflectionCoors = vec2(NDC.x, -NDC.y);

	float near = 0.1;
	float far  = 10000.0;
    float depth = texture(DepthMap, NDC).r;


    float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
    float waterDistance = 2.0 * near * far / (far + near - (2.0 * gl_FragCoord.z - 1.0) * (far - near));

    float waterDepth = floorDistance - waterDistance;
    float waterDepth5 = clamp(waterDepth / 25.0, 0.0, 1.0);
	float waterDepth20 = clamp(waterDepth / 40.0, 0.0, 1.0);


	vec2 Distortion1 = texture(DUDV_Map, vec2(TexCoords.x+MoveFactor, TexCoords.y)).rg * 0.1;
	Distortion1 = TexCoords + vec2(Distortion1.x, Distortion1.y+MoveFactor);

	vec2 Distortion = (texture(DUDV_Map, Distortion1).rg * 2.0 - 1.0) * WaveStrength * waterDepth20;


	ReflectionCoors += Distortion;
	RefractionCoors += Distortion;
	RefractionCoors = clamp(RefractionCoors, 0.001, 0.999);


	ReflectionCoors.x = clamp(ReflectionCoors.x,  0.001,  0.999);
	ReflectionCoors.y = clamp(ReflectionCoors.y, -0.999, -0.001);


	vec4 ReflectionColor = texture(ReflectionTex, ReflectionCoors);
	vec4 RefractionColor = texture(RefractionTex, RefractionCoors);




	



    // vec3 Normal = normalize(texture(NormalMap, Distortion).xyz * 2.0f - 1.0f);
    vec4 Norm = texture(NormalMap, Distortion);
    vec3 Normal = normalize(vec3(Norm.r * 2.0f - 1.0f, Norm.b * 3 ,Norm.g * 2.0f - 1.0f));
	float RefractieFactor = pow(dot(normalize(CameraDir), Normal), 0.3);


    // vec3 reflectDir = reflect(-LightDir, Normal);
    vec3 halfwayDir = normalize(CameraDir + LightDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), Shininess);
    vec4 Specular = SpecularStrength * spec * LightColor * waterDepth5;


	// FragColor  = vec4(waterDepth5);
	FragColor = mix(ReflectionColor , RefractionColor, RefractieFactor) + Specular;
    // float gamma = 2.2;
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));

	// FragColor = mix(FragColor , vec4(0, 0.3, 0.5, 1.0), 0.2); // Greenish Blueish Tint;
	FragColor.a = waterDepth5;



}