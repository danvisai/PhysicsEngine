#version 450 core
//input vertex attributes comes from our vao
// "layout(location = 0 )" must match the location specified in the vertex attribute pointer in our main cpp file
// "in" means this is an input variable "vec3" meanis this is vector of 3 floats and "apos" is the name of the 
// variable that will hold the vertex position data from the vertex buffer
layout(location = 0) in vec3 aPos; //input vertex position from vertex buffer

void main()
{
	gl_Position = vec4(aPos, 1.0); //convert the 3D vertex position to a 4D homogeneous coordinate by adding a w component of 1.0
}