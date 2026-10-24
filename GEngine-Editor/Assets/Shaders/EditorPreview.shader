Shader "EditorPreview"
{
    Pass
    {
        Cull Off
        DepthTest LessEqual
        DepthWrite On
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp Add
        ColorMask R G B A
        Tag EditorPreview
        #pragma vertex vert
        #pragma fragment frag
        Program
        {
            struct Input { float4 position : POSITION; float4 color : COLOR; };
            struct Output { float4 position : SV_POSITION; float4 color : COLOR; };
            Output vert(Input input) { Output output; output.position = input.position; output.color = input.color; return output; }
            float4 frag(Output input) : SV_Target { return input.color; }
        }
    }
}
