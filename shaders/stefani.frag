#version 430 core

out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec2 texCoord;




uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform sampler2D glossy0;
// uniform sampler2D opacity0;
// uniform sampler2D height0;

uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;


vec4 pointLight()
{	
	// used in two variables so I calculate it here to not have to do it twice
	vec3 lightVec = lightPos - crntPos;

	// intensity of light with respect to distance
	float dist = length(lightVec);
	float a = 1.00f;
	float b = 0.70f;
	float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

	// ambient lighting
	float ambient = 0.05f;

	// diffuse lighting
	// Normals are mapped from the range [0, 1] to the range [-1, 1]
	vec3 normal = normalize(texture(normal0, texCoord).xyz * 2.0f - 1.0f);
	
	vec3 lightDirection = normalize(lightVec);
	// vec3 lightDirection = normalize(TangentLightPos - TangentFragPos);

	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f)
	{
		float specularLight = 0.50f;
		// float specularLight = texture(glossy0, texCoord).r * 2.f;
	
		vec3 viewDirection = normalize(viewPos - crntPos);
		// vec3 viewDirection = normalize(TangentViewPos - TangentFragPos);
		
		vec3 halfwayVec = normalize(viewDirection + lightDirection);
		
		float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
		// float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), texture(glossy0, texCoord).r);

		specular = specAmount * specularLight;
	};

	vec4 albido = texture(diffuse0, texCoord);
	if (albido.a < 0.1f){discard;}

	return (albido * (diffuse * inten + ambient) + texture(specular0, texCoord).r * specular * inten) * lightColor;
}

vec4 direcLight()
{
	// ambient lighting
	float ambient = 0.50f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(vec3(1.0f, 1.0f, 0.0f));
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	float specular = 0.0f;
	if (diffuse != 0.0f)
	{
		float specularLight = 0.50f;
		vec3 viewDirection = normalize(viewPos - crntPos);
		vec3 halfwayVec = normalize(viewDirection + lightDirection);
		float specAmount = pow(max(dot(normal, halfwayVec), 0.0f), 16);
		specular = specAmount * specularLight;
	};

	vec4 albido = texture(diffuse0, texCoord);
	if (albido.a < 0.1f){discard;}

	return (albido * (diffuse + ambient) + texture(specular0, texCoord).r * specular) * lightColor;
}

vec4 paintStuff()
{
	return texture(glossy0, texCoord);

}

void main()
{
	// FragColor = pointLight();
	FragColor = direcLight();
	// FragColor = paintStuff();


}