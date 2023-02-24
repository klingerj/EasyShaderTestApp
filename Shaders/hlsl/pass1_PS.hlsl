struct PSInput
{
    [[vk::location(0)]] float4 Position : SV_POSITION;
    [[vk::location(1)]] float2 UV       : TEXCOORD0;
};

float4 main(PSInput Input) : SV_Target0
{
    float4 x = float4(0, 0, 0, 0);
    for (int i = 0; i < int(Input.UV.x * 10000.0f); ++i)
    {
        x[i % 4] += Input.UV.y;
    }
    return x;
}
