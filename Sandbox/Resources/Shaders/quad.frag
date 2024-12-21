#version 450

layout(location = 0) in vec4 v_Tint;
layout(location = 1) in vec2 v_TextureCoordinates;
//layout(location = 2) in flat int v_TextureIndex;
//layout(location = 3) in float v_TilingFactor;
//layout(location = 4) in flat int v_EntityID;

layout(binding = 1) uniform sampler2D u_Sampler; // make this an array, and index in with v_TextureIndex

layout(location = 0) out vec4 o_Colour;
//layout(location = 1) out int o_EntityID;

void main()
{
    o_Colour = v_Tint * texture(u_Sampler, v_TextureCoordinates);
    
   //vec4 textureColour = texture(u_Sampler, v_TextureCoordinates); // * v_TilingFactor);
   //if (textureColour.a == 0.0)
	//	discard;

    //o_EntityID = v_EntityID;
}