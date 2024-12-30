#version 450

layout(location = 0) in vec4 v_Colour;
layout(location = 1) in flat int v_EntityID;

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int o_EntityID;

void main()
{
    o_Colour = v_Colour;
    
    o_EntityID = v_EntityID;
}