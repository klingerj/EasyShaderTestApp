#define PositionDataType float4
#define UVDataType float2
#define NormalDataType float4

[[vk::binding(0, 1)]] StructuredBuffer<PositionDataType> PositionData;

struct VSOutput
{
    [[vk::location(0)]] float4 Position : SV_POSITION;
    [[vk::location(1)]] float2 UV       : TEXCOORD0;
};

VSOutput main(uint VertexIndex : SV_VertexID)
{
    float4 ModelPos = float4(PositionData.Load(VertexIndex).xyz, 1.0f);
    float2 UV = ModelPos.xy * 0.5f + 0.5f;
    UV.y = 1 - UV.y;

    VSOutput Out;
    Out.Position = ModelPos;
    Out.UV = UV;
    return Out;
}
