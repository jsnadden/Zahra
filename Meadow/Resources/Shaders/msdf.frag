#version 450

layout(location = 0) in vec2 v_TextureCoordinates;
layout(location = 1) in flat int v_EntityID;
layout(location = 2) in vec4 v_FillColour;
layout(location = 3) in vec4 v_BackgroundColour;
//layout(location = 4) in vec4 v_OutlineColour;
//layout(location = 5) in flat float v_LineWidth;
//layout(location = 6) in flat float v_AAWidth;

layout(set = 0, binding = 1) uniform sampler2D u_MSDFSampler;

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int o_EntityID;

float Median(float x, float y, float z)
{
    return max(min(x, y), min(max(x, y), z));
}

float pixelRange = 2.0f;

float ScreenPixelRange() {
    // TODO: should precalculate unitRange and send it here as a uniform
    vec2 unitRange = vec2(pixelRange)/vec2(textureSize(u_MSDFSampler, 0));
    vec2 screenPixelSize = vec2(1.0)/fwidth(v_TextureCoordinates);
    return max(0.5*dot(unitRange, screenPixelSize), 1.0);
}

void main()
{
    vec3 msdf = texture(u_MSDFSampler, v_TextureCoordinates).rgb;
    float pseudoDistance = Median(msdf.r, msdf.g, msdf.b);

    float screenTexelDistance = ScreenPixelRange()*(pseudoDistance - 0.5);
    float opacity = clamp(screenTexelDistance + 0.5, 0.0, 1.0);
    o_Colour = mix(v_BackgroundColour, v_FillColour, opacity);

    // TODO: add things like text outlines, glow effects, drop shadows etc.
    //float outer = 0.5 * (1.0 - v_LineWidth);
    //float inner = 1.0 - outer;
    //float outside = 1.0 - smoothstep(outer - v_AAWidth, outer, pseudoDistance);
    //float inside = smoothstep(inner, inner + v_AAWidth, pseudoDistance);

    //float blendAcross = smoothstep(outer, inner, pseudoDistance);
    //vec4 off = mix(v_BackgroundColour, v_FillColour, blendAcross);
    //o_Colour = mix(v_OutlineColour, off, inside + outside);

    if (o_Colour.a == 0)
		discard;
    
    o_EntityID = v_EntityID;
}