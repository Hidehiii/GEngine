Shader "FrameGraphColor"
{
    Pass
    {
        #pragma compute updateColor
        Program
        {
            RWByteAddressBuffer ColorData : register(u0);
            [numthreads(1, 1, 1)]
            void updateColor(uint3 id : SV_DispatchThreadID)
            {
                float4 color = asfloat(ColorData.Load4(0));
                color.r = color.r > 0.9f ? 0.15f : 0.95f;
                ColorData.Store4(0, asuint(color));
            }
        }
    }
}
