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
	vec4 textureColour = v_Colour;

	switch(int(v_TextureIndex))
	{
		case 0: textureColour *= texture(u_Textures[0], v_TextureCoord * v_TilingFactor); break;
		case 1: textureColour *= texture(u_Textures[1], v_TextureCoord * v_TilingFactor); break;
		case 2: textureColour *= texture(u_Textures[2], v_TextureCoord * v_TilingFactor); break;
		case 3: textureColour *= texture(u_Textures[3], v_TextureCoord * v_TilingFactor); break;
		case 4: textureColour *= texture(u_Textures[4], v_TextureCoord * v_TilingFactor); break;
		case 5: textureColour *= texture(u_Textures[5], v_TextureCoord * v_TilingFactor); break;
		case 6: textureColour *= texture(u_Textures[6], v_TextureCoord * v_TilingFactor); break;
		case 7: textureColour *= texture(u_Textures[7], v_TextureCoord * v_TilingFactor); break;
		case 8: textureColour *= texture(u_Textures[8], v_TextureCoord * v_TilingFactor); break;
		case 9: textureColour *= texture(u_Textures[9], v_TextureCoord * v_TilingFactor); break;
		case 10: textureColour *= texture(u_Textures[10], v_TextureCoord * v_TilingFactor); break;
		case 11: textureColour *= texture(u_Textures[11], v_TextureCoord * v_TilingFactor); break;
		case 12: textureColour *= texture(u_Textures[12], v_TextureCoord * v_TilingFactor); break;
		case 13: textureColour *= texture(u_Textures[13], v_TextureCoord * v_TilingFactor); break;
		case 14: textureColour *= texture(u_Textures[14], v_TextureCoord * v_TilingFactor); break;
		case 15: textureColour *= texture(u_Textures[15], v_TextureCoord * v_TilingFactor); break;
		case 16: textureColour *= texture(u_Textures[16], v_TextureCoord * v_TilingFactor); break;
		case 17: textureColour *= texture(u_Textures[17], v_TextureCoord * v_TilingFactor); break;
		case 18: textureColour *= texture(u_Textures[18], v_TextureCoord * v_TilingFactor); break;
		case 19: textureColour *= texture(u_Textures[19], v_TextureCoord * v_TilingFactor); break;
		case 20: textureColour *= texture(u_Textures[20], v_TextureCoord * v_TilingFactor); break;
		case 21: textureColour *= texture(u_Textures[21], v_TextureCoord * v_TilingFactor); break;
		case 22: textureColour *= texture(u_Textures[22], v_TextureCoord * v_TilingFactor); break;
		case 23: textureColour *= texture(u_Textures[23], v_TextureCoord * v_TilingFactor); break;
		case 24: textureColour *= texture(u_Textures[24], v_TextureCoord * v_TilingFactor); break;
		case 25: textureColour *= texture(u_Textures[25], v_TextureCoord * v_TilingFactor); break;
		case 26: textureColour *= texture(u_Textures[26], v_TextureCoord * v_TilingFactor); break;
		case 27: textureColour *= texture(u_Textures[27], v_TextureCoord * v_TilingFactor); break;
		case 28: textureColour *= texture(u_Textures[28], v_TextureCoord * v_TilingFactor); break;
		case 29: textureColour *= texture(u_Textures[29], v_TextureCoord * v_TilingFactor); break;
		case 30: textureColour *= texture(u_Textures[30], v_TextureCoord * v_TilingFactor); break;
		case 31: textureColour *= texture(u_Textures[31], v_TextureCoord * v_TilingFactor); break;
	}
	colour = textureColour;
}