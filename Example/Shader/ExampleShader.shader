Shader "Shader Name"
{
    Properties
    {

    }
    Pass
    {
        Cull
        DepthTest 
        DepthWrite
        Blend
        BlendOp
        ColorMask
        RenderPipeline

        #pragma vertex
        #pragma fragment

        Program
        {
            cbuffer
            {

            }

            struct VsInput
            {

            }

            struct VsOutput
            {

            }

            VsOutput vertex(VsInput IN)
            {
                VsOutput OUT;
                return OUT;
            }

            float4 fragment(VsOutput IN)
            {
                return float4(1.0, 1.0, 1.0, 1.0);
            }
        }
    }
}