#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout (location = 0) out vec4 v_Colour;

void main()
{
	v_Colour = a_Colour;	
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}



#type fragment
#version 450 core

layout(location = 0) out vec4 o_Colour;

layout (location = 0) in vec4 v_Colour;

void main()
{
	o_Colour = v_Colour;
}
