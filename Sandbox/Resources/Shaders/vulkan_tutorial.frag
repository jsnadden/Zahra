#version 450

layout(location = 0) in vec3 v_Colour;

layout(location = 0) out vec4 o_Colour;

void main(){
    o_Colour = vec4(v_Colour, 1.0);
}