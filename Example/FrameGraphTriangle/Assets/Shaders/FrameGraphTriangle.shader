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
                return float4(0.95f, 0.45f, 0.12f, 1.0f);
            }
        }
    }
}
