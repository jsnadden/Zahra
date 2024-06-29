#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_PVMatrix;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_PVMatrix * u_Transform * vec4(a_Position, 1.0);
}



#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

uniform vec4 u_Colour;

void main()
{
	colour = u_Colour;
}