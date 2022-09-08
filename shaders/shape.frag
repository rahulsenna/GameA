#version 330 core


// Ouput data
// out vec4 FragColor;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform vec4 Color;
void main(){
    FragColor = Color;
    BrightColor = Color;
    
    //    	FragColor = vec4(1,1,1,1);
}