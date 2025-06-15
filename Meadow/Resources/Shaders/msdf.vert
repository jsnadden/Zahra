#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextureCoordinates;
layout(location = 2) in int a_EntityID;
layout(location = 3) in vec4 a_FillColour;
layout(location = 4) in vec4 a_OutlineColour;
layout(location = 5) in vec4 a_BackgroundColour;
layout(location = 6) in float a_LineWidth;
layout(location = 7) in float a_AAWidth;

layout(set = 0, binding = 0) uniform Camera
{
    mat4 ViewProjection;
} u_Camera;

layout(location = 0) out vec2 v_TextureCoordinates;
layout(location = 1) out flat int v_EntityID;
layout(location = 2) out vec4 v_FillColour;
layout(location = 3) out vec4 v_OutlineColour;
layout(location = 4) out vec4 v_BackgroundColour;
layout(location = 5) out flat float v_LineWidth;
layout(location = 6) out flat float v_AAWidth;

void main()
{
    gl_Position = u_Camera.ViewProjection * vec4(a_Position, 1.0);
    v_TextureCoordinates = a_TextureCoordinates;
    v_EntityID = a_EntityID;
    v_FillColour = a_FillColour;
    v_OutlineColour = a_OutlineColour;
    v_BackgroundColour = a_BackgroundColour;
    v_LineWidth = a_LineWidth;
    v_AAWidth = a_AAWidth;
}