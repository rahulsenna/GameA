#version 430 core


out vec4 FragColor;


in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

in vec4 FragPosLightSpace;



uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform sampler2D glossy0;
uniform sampler2D height0;

uniform sampler2D shadowMap;


uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientVal;
uniform float specVal;
uniform float pcf;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(pcf * (1.0 - dot(normal, lightDir)), pcf);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
            // shadow += currentDepth - 0.00 > pcfDepth  ? 1.0 : 0.0;        

        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

vec4 pointLight()
{	
	// used in two variables so I calculate it here to not have to do it twice
	vec3 lightVec = lightPos - FragPos;

	// intensity of light with respect to distance
	float dist = length(lightVec);
	float a = 3.0;
	float b = 0.7;
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

	// ambient lighting
	float ambient = 0.20f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(viewPos - FragPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	return (texture(diffuse0, TexCoords) * (diffuse * inten + ambient) + texture(specular0, TexCoords).r * specular * inten) * lightColor;
}

vec4 direcLight()
{
     vec3 normal = normalize(texture(normal0, TexCoords).rgb * 2.0f - 1.0f);

    vec4 colorV4 = texture(diffuse0, TexCoords);
    if (colorV4.a < 0.2){discard;}

    vec3 color = colorV4.rgb;
    vec3 ambient = ambientVal * color;
    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);

    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * color;

    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(viewDir + lightDir);

    float shininess = max((texture(glossy0, TexCoords).r*1080.f), 0.0);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 specular = vec3(specVal) * spec;
    // return vec4(ambient + diffuse + specular, 1.0) * lightColor;
    float shadow = ShadowCalculation(FragPosLightSpace);
    ambient  *= lightColor;
    diffuse  *= lightColor;
    specular *= lightColor;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    return vec4(lighting, 1.0);
}

vec4 spotLight()
{
	// controls how big the area that is lit up is
	float outerCone = 0.90f;
	float innerCone = 0.95f;

	// ambient lighting
	float ambient = 0.20f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - FragPos);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(viewPos - FragPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	// calculates the intensity of the FragPos based on its angle to the center of the light cone
	float angle = dot(vec3(0.0f, -1.0f, 0.0f), -lightDirection);
	float inten = clamp((angle - outerCone) / (innerCone - outerCone), 0.0f, 1.0f);

	return (texture(diffuse0, TexCoords) * (diffuse * inten + ambient) + texture(specular0, TexCoords).r * specular * inten) * lightColor;
}

float near = 0.1f;
float far = 100.0f;


float linearizeDepth(float depth)
{
    return(2.0f * near *far)/ (far + near - (depth *2.f -1.f) * (far - near));
}

float logisticDepth(float depth, float steepness, float offset)
{
    float zVal = linearizeDepth(depth);
    return (1/ (1 +  exp(-steepness *(zVal - offset))));
}

void main()
{
	// outputs final color
	// float depth = logisticDepth(gl_FragCoord.z, 0.115f, 15.0f);
	FragColor = direcLight();
	// FragColor = vec4(1);
}