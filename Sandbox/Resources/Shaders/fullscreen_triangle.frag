#version 450

layout(location = 0) in vec2 v_TextureCoordinates;

layout(binding = 0) uniform sampler2D u_Sampler;

layout(location = 0) out vec4 o_Colour;

void main()
{
    o_Colour = texture(u_Sampler, v_TextureCoordinates);
}