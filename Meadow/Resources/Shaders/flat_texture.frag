#version 450

layout(location = 0) in vec4 v_Tint;
layout(location = 1) in vec2 v_TextureCoordinates;
layout(location = 2) in flat int v_TextureIndex;
layout(location = 3) in flat float v_TilingFactor;
layout(location = 4) in flat int v_EntityID;

layout(set = 0, binding = 1) uniform sampler2D u_Sampler[32];

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int o_EntityID;

void main()
{
    o_Colour = v_Tint * texture(u_Sampler[v_TextureIndex], v_TextureCoordinates * v_TilingFactor);

    o_EntityID = v_EntityID;
}