#version 450

layout(location = 0) in vec2 v_TextureCoordinates;
layout(location = 1) in flat int v_EntityID;
layout(location = 2) in vec4 v_FillColour;
layout(location = 3) in vec4 v_OutlineColour;
layout(location = 4) in vec4 v_BackgroundColour;
layout(location = 5) in flat float v_LineWidth;
layout(location = 6) in flat float v_AAWidth;

layout(set = 0, binding = 1) uniform sampler2D u_MSDFSampler;

layout(location = 0) out vec4 o_Colour;
layout(location = 1) out int o_EntityID;

float Median(float x, float y, float z)
{
  return max(min(x,y), min(max(x,y),z));
}

void main()
{
    vec3 msdf = texture(u_MSDFSampler, v_TextureCoordinates).rgb;
    float df = Median(msdf.r, msdf.g, msdf.b);

    float fill = smoothstep(v_LineWidth, v_AAWidth + v_LineWidth, df);
    float line = smoothstep(-(v_AAWidth + v_LineWidth), -v_LineWidth, df)
      * smoothstep(-(v_AAWidth + v_LineWidth), -v_LineWidth, -df);
    float bg = smoothstep(v_LineWidth, v_AAWidth + v_LineWidth, -df);
    
    o_Colour = line * v_OutlineColour;

    if (o_Colour.a == 0)
		discard;

    o_EntityID = v_EntityID;
}