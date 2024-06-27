#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextureCoord;

uniform mat4 u_PVMatrix;
uniform mat4 u_Transform;
			
out vec2 v_TextureCoord;

void main()
{
	v_TextureCoord = a_TextureCoord;
	gl_Position = u_PVMatrix * u_Transform * vec4(a_Position, 1.0);
}



#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_TextureCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Colour;

void main()
{
	colour = texture(u_Texture, v_TextureCoord) * u_Colour;
}