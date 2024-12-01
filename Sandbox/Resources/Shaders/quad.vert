#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Tint;
layout (location = 2) in vec2 a_TextureCoordinates;
layout (location = 3) in int a_TextureIndex;
layout (location = 4) in float a_TilingFactor;
//layout (location = 5) in int a_EntityID;

layout(binding = 0) uniform Camera
{
    mat4 ViewProjection;
} u_Camera;

layout(location = 0) out vec4 v_Tint;
layout(location = 1) out vec2 v_TextureCoordinates;
layout(location = 2) out flat int v_TextureIndex;
layout(location = 3) out float v_TilingFactor;
//layout(location = 4) out flat int v_EntityID;

void main()
{
    gl_Position = u_Camera.ViewProjection * vec4(a_Position, 1.0);
    v_Tint = a_Tint;
    v_TextureCoordinates = a_TextureCoordinates;
    v_TextureIndex = a_TextureIndex;
    v_TilingFactor = a_TilingFactor;
   // v_EntityID = a_EntityID;
}