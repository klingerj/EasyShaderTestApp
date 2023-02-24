struct PushConstantData
{
    float2 Scale;
    float2 Translate;
};

[[vk::push_constant]]
PushConstantData PushConstants;

#define PositionDataType float2
#define UVDataType float2
#define ColorDataType uint

[[vk::binding(0, 1)]] StructuredBuffer<PositionDataType> PositionData;
[[vk::binding(1, 1)]] StructuredBuffer<UVDataType>       UVData;
[[vk::binding(2, 1)]] StructuredBuffer<ColorDataType>    ColorData;

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_POSITION;
    [[vk::location(1)]] float2 UV       : TEXCOORD0;
    [[vk::location(2)]] float4 Color    : COLOR;
};

float srgbToLinear(float srgb)
{
    if (srgb <= 0.04045)
    {
        return srgb / 12.92;
    }
    else
    {
        return pow((srgb + 0.055) / 1.055, 2.4);
    }
}

float3 srgbToLinear(float3 srgb)
{
    return float3(srgbToLinear(srgb.r), srgbToLinear(srgb.g), srgbToLinear(srgb.b));
}

VSOutput main(uint VertexIndex : SV_VertexID, uint InstanceIndex : SV_InstanceID)
{
    float2 ModelPos = PositionData.Load(VertexIndex).xy;
    float2 FinalPos = ModelPos * PushConstants.Scale * 0.5;
    FinalPos.y = 1 - FinalPos.y; // For Vulkan - TODO don't do this in shader, or just ifdef it and move on
    FinalPos *= 2.0;
    FinalPos += PushConstants.Translate;

    float2 UV = UVData.Load(VertexIndex);

    uint Color = ColorData.Load(VertexIndex);
    float4 ColorUnpacked;
    float denom = 1.0 / 255.0;
    ColorUnpacked.r = ((Color >>  0) & 0xFF) * denom;
    ColorUnpacked.g = ((Color >>  8) & 0xFF) * denom;
    ColorUnpacked.b = ((Color >> 16) & 0xFF) * denom;
    ColorUnpacked.a = ((Color >> 24) & 0xFF) * denom;

    VSOutput Out;
    Out.Position = float4(FinalPos, 0, 1);
    Out.UV = UV;
    Out.Color = float4(srgbToLinear(ColorUnpacked.rgb), ColorUnpacked.a);
    return Out;
}
