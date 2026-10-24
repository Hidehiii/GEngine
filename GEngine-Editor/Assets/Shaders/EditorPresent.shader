Shader "EditorPresent"
{
    Pass
    {
        Cull Off
        DepthTest Always
        DepthWrite Off
        Blend One Zero
        BlendOp None
        ColorMask R G B A
        Tag EditorPresent
        #pragma vertex vert
        #pragma fragment frag
        Program
        {
#ifdef __spirv__
            [[vk::combinedImageSampler]]
#endif
            Texture2D GE_PRESENT_IMGUI : register(t0);
#ifdef __spirv__
            [[vk::combinedImageSampler]]
#endif
            SamplerState GE_PRESENT_IMGUI_Sampler : register(s0);
            struct Input { float4 position : POSITION; float2 uv : TEXCOORD0; };
            struct Output { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
            Output vert(Input input) { Output output; output.position = input.position; output.uv = input.uv; return output; }
            float4 frag(Output input) : SV_Target { return GE_PRESENT_IMGUI.Sample(GE_PRESENT_IMGUI_Sampler, input.uv); }
        }
    }
}
