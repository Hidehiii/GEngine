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
        Tag Default

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            cbuffer material
            {
                int _SomeInt;
                float _SomeFloat;
                float2 _SomeVector;
                float2 _SomeVector2;
                float3 _Color;
                float4 _Color1;
                float4x4 _mat4;
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
                float4x4 mat4 : TEXCOORD0;
            };

            VsOutput vert(VsInput IN)
            {
                VsOutput OUT;
                OUT.pos.xyz = IN.vertex.xyz * _Color;
                OUT.pos.w = 1.0;
                OUT.mat4 = _mat4;
                OUT.mat4[0][0] = _SomeFloat;
                int i = _SomeInt;
                float2 v2 = _SomeVector + _SomeVector2;
                return OUT;
            }

            float4 frag(VsOutput IN) : SV_Target
            {
                return _MainTex.Sample(_MainTex_sampler, IN.pos.xy) * _Color1;
            }
        }
    }
}