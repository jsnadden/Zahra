#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_PVMatrix;
			
out vec4 v_Colour;
out vec2 v_TextureCoord;
out float v_TextureIndex;
out float v_TilingFactor;

void main()
{
	v_Colour = a_Colour;
	v_TextureCoord = a_TextureCoord;
	v_TextureIndex = a_TextureIndex;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_PVMatrix * vec4(a_Position, 1.0);
}



#type fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec4 v_Colour;
in vec2 v_TextureCoord;
in float v_TextureIndex;
in float v_TilingFactor;

uniform sampler2D u_Textures[32];

void main()
{
	colour = texture(u_Textures[int(v_TextureIndex)], v_TextureCoord * v_TilingFactor) * v_Colour;
}