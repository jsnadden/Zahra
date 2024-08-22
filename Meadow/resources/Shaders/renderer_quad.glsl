#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TextureCoord;
layout(location = 3) in float a_TextureIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Colour;
	vec2 TextureCoord;
	float TilingFactor;
};
	
layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat float v_TextureIndex;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.Colour = a_Colour;
	Output.TextureCoord = a_TextureCoord;
	Output.TilingFactor = a_TilingFactor;
	
	v_TextureIndex = a_TextureIndex;
	v_EntityID = a_EntityID;
	
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}



#type fragment
#version 450 core

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec4 Colour;
	vec2 TextureCoord;
	float TilingFactor;
};

layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat float v_TextureIndex;
layout (location = 4) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	vec4 textureColour = Input.Colour;

	switch(int(v_TextureIndex))
	{
		case  0: textureColour *= texture(u_Textures[0],  Input.TextureCoord * Input.TilingFactor); break;
		case  1: textureColour *= texture(u_Textures[1],  Input.TextureCoord * Input.TilingFactor); break;
		case  2: textureColour *= texture(u_Textures[2],  Input.TextureCoord * Input.TilingFactor); break;
		case  3: textureColour *= texture(u_Textures[3],  Input.TextureCoord * Input.TilingFactor); break;
		case  4: textureColour *= texture(u_Textures[4],  Input.TextureCoord * Input.TilingFactor); break;
		case  5: textureColour *= texture(u_Textures[5],  Input.TextureCoord * Input.TilingFactor); break;
		case  6: textureColour *= texture(u_Textures[6],  Input.TextureCoord * Input.TilingFactor); break;
		case  7: textureColour *= texture(u_Textures[7],  Input.TextureCoord * Input.TilingFactor); break;
		case  8: textureColour *= texture(u_Textures[8],  Input.TextureCoord * Input.TilingFactor); break;
		case  9: textureColour *= texture(u_Textures[9],  Input.TextureCoord * Input.TilingFactor); break;
		case 10: textureColour *= texture(u_Textures[10], Input.TextureCoord * Input.TilingFactor); break;
		case 11: textureColour *= texture(u_Textures[11], Input.TextureCoord * Input.TilingFactor); break;
		case 12: textureColour *= texture(u_Textures[12], Input.TextureCoord * Input.TilingFactor); break;
		case 13: textureColour *= texture(u_Textures[13], Input.TextureCoord * Input.TilingFactor); break;
		case 14: textureColour *= texture(u_Textures[14], Input.TextureCoord * Input.TilingFactor); break;
		case 15: textureColour *= texture(u_Textures[15], Input.TextureCoord * Input.TilingFactor); break;
		case 16: textureColour *= texture(u_Textures[16], Input.TextureCoord * Input.TilingFactor); break;
		case 17: textureColour *= texture(u_Textures[17], Input.TextureCoord * Input.TilingFactor); break;
		case 18: textureColour *= texture(u_Textures[18], Input.TextureCoord * Input.TilingFactor); break;
		case 19: textureColour *= texture(u_Textures[19], Input.TextureCoord * Input.TilingFactor); break;
		case 20: textureColour *= texture(u_Textures[20], Input.TextureCoord * Input.TilingFactor); break;
		case 21: textureColour *= texture(u_Textures[21], Input.TextureCoord * Input.TilingFactor); break;
		case 22: textureColour *= texture(u_Textures[22], Input.TextureCoord * Input.TilingFactor); break;
		case 23: textureColour *= texture(u_Textures[23], Input.TextureCoord * Input.TilingFactor); break;
		case 24: textureColour *= texture(u_Textures[24], Input.TextureCoord * Input.TilingFactor); break;
		case 25: textureColour *= texture(u_Textures[25], Input.TextureCoord * Input.TilingFactor); break;
		case 26: textureColour *= texture(u_Textures[26], Input.TextureCoord * Input.TilingFactor); break;
		case 27: textureColour *= texture(u_Textures[27], Input.TextureCoord * Input.TilingFactor); break;
		case 28: textureColour *= texture(u_Textures[28], Input.TextureCoord * Input.TilingFactor); break;
		case 29: textureColour *= texture(u_Textures[29], Input.TextureCoord * Input.TilingFactor); break;
		case 30: textureColour *= texture(u_Textures[30], Input.TextureCoord * Input.TilingFactor); break;
		case 31: textureColour *= texture(u_Textures[31], Input.TextureCoord * Input.TilingFactor); break;
	}
	o_Colour = textureColour;

	o_EntityID = v_EntityID; 
}
