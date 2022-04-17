#version 430 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 texCoord;


in vec3 ViewDir;
in vec3 LightDir;

// in vec3 TangentLightPos;
// in vec3 TangentViewPos;
// in vec3 TangentFragPos;

in vec4 FragPosLightSpace;


uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform sampler2D glossy0;
uniform sampler2D opacity0;
uniform sampler2D diffuse1;
uniform sampler2D height0;

uniform sampler2D shadowMap;


uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ambientVal;
uniform float specVal;
uniform float alphaEPSI;
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
    vec3 LightDir = normalize(lightPos - FragPos);
    float bias = max(pcf * (1.0 - dot(normal, LightDir)), pcf);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
    shadow = 0.0;

    return shadow;
}


vec4 pointLight()
{
    // used in two variables so I calculate it here to not have to do it twice
    vec3 lightVec = lightPos - FragPos;

    // intensity of light with respect to distance
    float dist = length(lightVec);
    float a = 1.00f;
    float b = 0.70f;
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    // ambient lighting
    float ambient = 0.1f;

    // diffuse lighting
    // Normals are mapped from the range [0, 1] to the range [-1, 1]
    vec3 normal = normalize(texture(normal0, texCoord).xyz * 2.0f - 1.0f);

    // vec3 LightDir = normalize(lightVec);
    // vec3 LightDir = normalize(TangentLightPos - TangentFragPos);

    float diffuse = max(dot(normal, LightDir), 0.0f);

    // specular lighting
    float specular = 0.0f;
    if (diffuse != 0.0f)
    {
        float specularLight = 0.50f;
        // float specularLight = texture(glossy0, texCoord).r * 2.f;

        // vec3 ViewDir = normalize(viewPos - FragPos);
        // vec3 ViewDir = normalize(TangentViewPos - TangentFragPos);

        vec3 halfwayDir = normalize(ViewDir + LightDir);
        int shininess = 16;
        // shininess = texture(glossy0, texCoord).r


        float specAmount = pow(max(dot(normal, halfwayDir), 0.0f), shininess);


        specular = specAmount * specularLight;
    };

    vec4 albido = texture(diffuse0, texCoord);
    if (albido.a < 0.1f){ discard; }

    return (albido * (diffuse * inten + ambient) + texture(specular0, texCoord).r * specular * inten) * lightColor;
}
// vec4 albido = texture(diffuse0, texCoord);
// if (albido.a < 0.1f){discard;}

vec4 direcLight()
{
    vec3 normal = normalize(texture(normal0, texCoord).rgb * 2.0f - 1.0f);

    vec4 colorV4 = texture(diffuse0, texCoord);
    if (colorV4.a < 0.2){ discard; }

    vec3 color = colorV4.rgb;
    vec3 ambient = ambientVal * color;
    // vec3 LightDir = normalize(TangentLightPos - TangentFragPos);

    float diff = max(dot(normal, LightDir), 0.0f);
    vec3 diffuse = diff * color;

    // vec3 ViewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 reflectDir = reflect(-LightDir, normal);
    vec3 halfwayDir = normalize(ViewDir + LightDir);

    float shininess = max((texture(glossy0, texCoord).r*1080.f), 0.0);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 specular = vec3(specVal) * spec;
    // return vec4(ambient + diffuse + specular, 1.0) * lightColor;
    float shadow = ShadowCalculation(FragPosLightSpace);
    ambient  *= lightColor.rgb;
    diffuse  *= lightColor.rgb;
    specular *= lightColor.rgb;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    return vec4(lighting, 1.0);
}

vec4 paintStuff()
{
    return texture(diffuse1, texCoord);

}

void main()
{

    // vec4 result = pointLight();
    // result += direcLight();
    // FragColor = result;
        FragColor = direcLight();
//    FragColor = vec4(texture(diffuse0, texCoord));
}