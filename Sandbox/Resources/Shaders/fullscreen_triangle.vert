#version 450

vec2 positions[3] = vec2[]
(
    vec2(0.0, 0.0),
    vec2(2.0, 0.0),
    vec2(0.0, 2.0)
);

layout(location = 0) out vec2 v_TextureCoordinates;

void main()
{
    v_TextureCoordinates = positions[gl_VertexIndex];
    gl_Position = vec4(2 * v_TextureCoordinates - vec2(1.0, 1.0f), 0.0, 1.0);
}