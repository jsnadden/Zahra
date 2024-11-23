#version 450

layout(location = 0) in vec3 v_Colour;
layout(location = 1) in vec2 v_TexCoords;

layout(binding = 1) uniform sampler2D u_Texture;

layout(location = 0) out vec4 o_Colour;

void main()
{
    o_Colour = vec4(v_Colour, 1.0) * texture(u_Texture, 2.0 * v_TexCoords);
}