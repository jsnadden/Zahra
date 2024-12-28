#version 450

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec2 a_LocalPosition;
layout(location = 2) in vec4 a_Colour;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
//layout (location = 5) in int a_EntityID;

layout(binding = 0) uniform Camera
{
    mat4 ViewProjection;
} u_Camera;

layout(location = 0) out vec2 v_LocalPosition;
layout(location = 1) out vec4 v_Colour;
layout(location = 2) out float v_Thickness;
layout(location = 3) out float v_Fade;
//layout(location = 4) out flat int v_EntityID;

void main()
{
    gl_Position = u_Camera.ViewProjection * vec4(a_WorldPosition, 1.0);
    v_LocalPosition = a_LocalPosition;
    v_Colour = a_Colour;
	v_Thickness = a_Thickness;
	v_Fade = a_Fade;
   // v_EntityID = a_EntityID;
}