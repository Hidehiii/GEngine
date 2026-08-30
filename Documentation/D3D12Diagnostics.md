# D3D12 diagnostics and troubleshooting

## Enable validation

The D3D12 debug layer is opt-in in Debug builds. Enable it for a diagnostic run:

```powershell
$env:GENGINE_D3D12_DEBUG_LAYER = '1'
.\Triangle.exe
```

The engine enables the D3D12 debug layer before device creation and drains warnings/errors from a bounded info queue. The queue deliberately filters informational messages and has a finite capacity so validation cannot grow memory without limit during a render loop.

Do not enable it for ordinary profiling; validation changes timing and may report known third-party messages.

## RenderDoc capture checklist

For an empty draw, inspect the pipeline in this order:

1. **Event list:** confirm a `DrawInstanced` or `DrawIndexedInstanced` event exists.
2. **Input Assembler:** verify the vertex-buffer address, stride, semantic names and DXGI format.
3. **Vertex output:** check clip-space position and `w`. A visible triangle normally has `w = 1` and x/y values within the clip volume.
4. **Rasterizer:** confirm culling, scissor and viewport. For a baseline triangle use `Cull Off`.
5. **Depth/stencil and blend:** use `DepthTest Always`, `DepthWrite Off`, full `ColorMask`, and no blending to isolate output.
6. **Output Merger:** verify the bound RTV is the current swap-chain back buffer and the target was transitioned to render-target state.
7. **Final resource state:** confirm it returns to Present before `IDXGISwapChain::Present`.

## Common symptoms

| Symptom | Likely cause | First check |
| --- | --- | --- |
| `float3` appears as multiple scalar vertices | Reflection derived an incorrect input format/stride. | Vertex signature Mask and generated `DXGI_FORMAT`. |
| Correct VS positions but no fragments | Face culling, depth/stencil, scissor, or color-mask state removes output. | Use `Cull Off`, `DepthTest Always`, full color mask. |
| Blank window with no diagnostic | `Present` failure was ignored or no draw is recorded. | Check `Present` HRESULT and RenderDoc event list. |
| Memory grows every frame | Command allocators or descriptors are not reused/freed after their fences complete. | Reset the per-frame allocator only after waiting for its fence. |
| `GEngine.pch` is locked | Two builds share the intermediate directory. | Stop the duplicate build; use one build invocation. |
| `vc143.pdb` C1041 | Parallel compilers write a shared PDB without `/FS`. | Regenerate with the updated Premake setting or ensure `/FS` is present. |

## D3D12 lifetime rules

- A command list may be reset only after it has been closed.
- Its allocator may be reset only after the GPU has completed commands recorded with that allocator.
- Keep shader bytecode alive for as long as a PSO uses its `D3D12_SHADER_BYTECODE` pointers.
- Keep descriptor-range arrays alive until `D3D12SerializeRootSignature` finishes.
- Check HRESULT values from device, swap-chain, command-list and resource-creation calls; do not discard failures from `Present`.

## Triangle baseline

`Example/Triangle` is the intended first D3D12 smoke test. Its shader consumes one `POSITION` `float3`, writes an opaque cyan `SV_Target`, disables culling via `Cull Off`, uses an always-pass depth test and does not write depth. If this sample fails, fix the rendering foundation before debugging higher-level scene rendering.
