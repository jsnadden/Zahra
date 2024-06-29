#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TextureCoord;

uniform mat4 u_PVMatrix;
			
out vec4 v_Colour;
out vec2 v_TextureCoord;

void main()
{
	v_Colour = a_Colour;
	v_TextureCoord = a_TextureCoord;
	gl_Position = u_PVMatrix * vec4(a_Position, 1.0);
}



#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec4 v_Colour;
in vec2 v_TextureCoord;

uniform vec4 u_Colour;
uniform sampler2D u_Texture;
uniform float u_Tiling;

void main()
{
	// TODO: re-implement textures
	//colour = texture(u_Texture, v_TextureCoord * u_Tiling) * v_Colour;
	colour = v_Colour;
}