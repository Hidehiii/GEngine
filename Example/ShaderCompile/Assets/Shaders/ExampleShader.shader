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
                OUT.pos = IN.vertex;

                return OUT;
            }

            float4 frag(VsOutput IN) : SV_Target
            {
                return _Color;
            }
        }
    }
}