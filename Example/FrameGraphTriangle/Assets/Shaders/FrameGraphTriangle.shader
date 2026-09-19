Shader "FrameGraphTriangle"
{
    Pass
    {
        Cull Off
        DepthTest Always
        DepthWrite Off
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp None
        ColorMask R G B A
        Tag FrameGraphTriangle

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            RWByteAddressBuffer ColorData : register(u0);
            struct VsInput
            {
                float3 position : POSITION;
            };

            struct VsOutput
            {
                float4 position : SV_POSITION;
            };

            VsOutput vert(VsInput input)
            {
                VsOutput output;
                output.position = float4(input.position, 1.0f);
                return output;
            }

            float4 frag(VsOutput input) : SV_Target
            {
                return asfloat(ColorData.Load4(0));
            }
        }
    }
}
