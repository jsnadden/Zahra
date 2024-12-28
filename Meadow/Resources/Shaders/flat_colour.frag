#version 450

layout(location = 0) in vec4 v_Colour;

layout(location = 0) out vec4 o_Colour;

void main()
{
    o_Colour = v_Colour;
}