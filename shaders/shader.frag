#version 430 core

out vec4 FragColor;

in vec3 currentPos;
in vec3 normal;
in vec3 color;
in vec2 textCoord;



uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D diffuse0;
uniform sampler2D specular0;


vec4 spotLight()
{
   float outerCone = 0.90f;
   float innerCone = 0.95f;

   float ambient = 0.2f;
   vec3 Normal = normalize(normal);
   vec3 lightDirection = normalize(lightPos - currentPos);

   float diffuse = max(dot(Normal, lightDirection), 0.0f);

   float specularLight = 0.5f;
   vec3 viewDirection = normalize(viewPos - currentPos);
   vec3 reflectDirection = reflect(-lightDirection, Normal);
   float specAmount = pow(max(dot(viewDirection, reflectDirection), 0.f), 16);

   float angle = dot(vec3(0.f, -1.0f, 0.0f), -lightDirection);
   float intensity = clamp((angle - outerCone)/ (innerCone - outerCone), 0.f, 1.f);

   float specular = specularLight * specAmount * texture(specular0, textCoord).r * intensity;

   return(texture(diffuse0, textCoord) * lightColor * (diffuse * intensity + ambient) + specular);
}

vec4 pointLight()
{
   vec3 lightVec = lightPos - currentPos;
   float dist = length(lightVec);

   float a = 3.f;
   float b = 0.7f;
   float intensity = 1.0f / (a * dist * dist + b * dist + 1.f);


   float ambient = 0.2f;
   vec3 Normal = normalize(normal);
   vec3 lightDirection = normalize(lightVec);

   float diffuse = max(dot(Normal, lightDirection), 0.0f);

   float specularLight = 0.5f;
   vec3 viewDirection = normalize(viewPos - currentPos);
   vec3 reflectDirection = reflect(-lightDirection, Normal);
   float specAmount = pow(max(dot(viewDirection, reflectDirection), 0.f), 16);

   float specular = specularLight * specAmount * texture(specular0, textCoord).r * intensity;

   return(texture(diffuse0, textCoord) * lightColor * (diffuse * intensity + ambient) + specular);
}

vec4 directLight()
{

   float ambient = 0.2f;
   vec3 Normal = normalize(normal);

   vec3 lightDirection = normalize(vec3(1.0f, 1.0f,0.0f));

   float diffuse = max(dot(Normal, lightDirection), 0.0f);

   float specularLight = 0.5f;
   vec3 viewDirection = normalize(viewPos - currentPos);
   vec3 reflectDirection = reflect(-lightDirection, Normal);
   float specAmount = pow(max(dot(viewDirection, reflectDirection), 0.f), 16);

   float specular = specularLight * specAmount * texture(specular0, textCoord).r;

   return(texture(diffuse0, textCoord) * lightColor * (diffuse + ambient) + specular);
}

void main()
{
   FragColor = directLight();
}