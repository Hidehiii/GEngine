Shader "SceneImageColor"
{
    Pass
    {
        Cull Off
        DepthTest Always
        DepthWrite Off
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp None
        ColorMask R G B A
        Tag SceneImageColor

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            cbuffer material
            {
                float4x4 GE_MATRIX_M;
                float4 GE_IMAGE_COLOR;
            };

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
                output.position = mul(GE_MATRIX_M, float4(input.position, 1.0f));
                return output;
            }

            float4 frag(VsOutput input) : SV_Target
            {
                return GE_IMAGE_COLOR;
            }
        }
    }
}
