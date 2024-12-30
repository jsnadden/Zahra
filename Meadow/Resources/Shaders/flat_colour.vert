#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in int a_EntityID;

layout(binding = 0) uniform Camera
{
    mat4 ViewProjection;
} u_Camera;

layout(location = 0) out vec4 v_Colour;
layout(location = 1) out flat int v_EntityID;

void main()
{
    gl_Position = u_Camera.ViewProjection * vec4(a_Position, 1.0);
    v_Colour = a_Colour;
    v_EntityID = a_EntityID;
}