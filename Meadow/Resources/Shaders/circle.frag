#version 450

layout(location = 0) in vec2 v_LocalPosition;
layout(location = 1) in vec4 v_Colour;
layout(location = 2) in float v_Thickness;
layout(location = 3) in float v_Fade;
//layout(location = 4) in flat int v_EntityID;

layout(location = 0) out vec4 o_Colour;
//layout(location = 1) out int o_EntityID;

void main()
{
    o_Colour = v_Colour;

    // Apply a C1 cutoff function to the alpha channel, discarding pixels away from a smoothed annulus
    float x = 1.0 - length(v_LocalPosition);
    o_Colour.a *= smoothstep(0.0, v_Fade, x);
    o_Colour.a *= smoothstep(v_Thickness + v_Fade, v_Thickness, x);

	if (o_Colour.a == 0)
		discard;

    //o_EntityID = v_EntityID;
}