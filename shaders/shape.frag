#version 330 core


// Ouput data
out vec4 FragColor;


uniform vec4 Color;
void main(){
    FragColor = Color;
    //    	FragColor = vec4(1,1,1,1);
}