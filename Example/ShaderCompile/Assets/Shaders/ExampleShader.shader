Shader "ExampleShader"
{
    Pass
    {
        Cull Back
        DepthTest LEqual
        DepthWrite On
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp None
        ColorMask R G B A
        Tag Triangle

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            cbuffer material
            {
                float4 _Color;
            };

            cbuffer mat
            {
                float4 _Color1;
            };

            struct VsInput
            {
                float3 vertex : POSITION;
                float2 yz : COLOR0;
            };

            struct VsOutput
            {
                float4 pos : SV_POSITION;
            };

            VsOutput vert(VsInput IN)
            {
                VsOutput OUT;;
                OUT.pos = float4(IN.vertex, 1.0f);

                return OUT;
            }

            float4 frag(VsOutput IN) : SV_Target
            {
                return float4(_Color.r, _Color1.g, 0.0f, 1.0f);
            }
        }
    }
}