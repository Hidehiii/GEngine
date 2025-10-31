Shader "ExampleShader"
{
    Properties
    {
        Sampler2D _MainTex;
    }
    Pass
    {
        Cull Back
        DepthTest LEqual
        DepthWrite On
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp None
        ColorMask R G B A
        Tag Default

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            cbuffer material
            {
                float3 _Color;
                float4 _Color1;
            };

            Texture2D _MainTex;
            sampler _MainTex_sampler;

            struct VsInput
            {
                float4 vertex : POSITION;
            };

            struct VsOutput
            {
                float4 pos : SV_POSITION;
            };

            VsOutput vert(VsInput IN)
            {
                VsOutput OUT;
                OUT.pos.xyz = IN.vertex.xyz * _Color;
                return OUT;
            }

            float4 frag(VsOutput IN) : SV_Target
            {
                return _MainTex.Sample(_MainTex_sampler, IN.pos.xy) * _Color1;
            }
        }
    }
}