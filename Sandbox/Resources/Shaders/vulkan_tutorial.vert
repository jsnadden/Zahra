#version 450

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec3 a_Colour;
layout (location = 2) in vec2 a_TexCoords;

layout(binding = 0) uniform Matrices
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
} u_Matrix;

layout(location = 0) out vec3 v_Colour;
layout(location = 1) out vec2 v_TexCoords;

void main()
{
    gl_Position = u_Matrix.Projection * u_Matrix.View * u_Matrix.Model * vec4(a_Position, 0.0, 1.0);
    v_Colour = a_Colour;
    v_TexCoords = a_TexCoords;    
}