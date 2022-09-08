#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;


// Imports the camera matrix
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main(){

	
	gl_Position = projection * view *model * vec4(aPos, 1.0);



}

