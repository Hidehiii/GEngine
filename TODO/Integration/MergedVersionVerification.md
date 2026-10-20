## Fresh merged-version verification

Debug, VS2022, 2026-09-19: OpenGL, Vulkan shared queues, Vulkan dedicated queues,
and D3D12 each completed 120 frames, passed the frame-119 compute readback and
exited normally with code 0. D3D12 debug layer was enabled for this check.
Vulkan dedicated queues selected graphics=0, compute=2, transfer=5. No Vulkan
validation errors or D3D12 errors were found in captured output. Known warnings:
OpenGL shader-state recompilation / one-time readback transfer, and D3D12
storage-buffer initial UAV state being ignored in favor of COMMON.

Debug replacement stress ran all three APIs for 65 seconds concurrently,
including material/buffer/pipeline/render-pass replacement every 30 frames.
Vulkan used dedicated queues. D3D12 debug layer was disabled only for the
normal-runtime memory baseline (the separate diagnostic run above enabled it).

| Backend | Frames | Private bytes at 10 s | Private bytes at 60 s | Exit |
| --- | ---: | ---: | ---: | ---: |
| OpenGL | 193170 | 171413504 | 171253760 | 0 |
| Vulkan | 39271 | 415576064 | 415502336 | 0 |
| D3D12 | 50500 | 166256640 | 166567936 | 0 |

Samples were taken every 10 seconds. These bounded observations showed no
sustained allocation growth; they are not proof of indefinite leak freedom.
All three readbacks passed, stderr was empty, and no validation errors were
found. Logs remain locally under `.integration/runs-20260919/<backend>`.

Release, VS2022, 2026-09-20: OpenGL, Vulkan shared queues, Vulkan dedicated
queues and D3D12 each completed 120 frames, passed compute readback and exited
with code 0. All captured stderr files were empty. Release does not provide the
Debug validation-layer coverage described above.

Remaining checks: manual visible output/resize/minimize,
extended D3D12 validation-layer memory behavior, and VS2026 Debug compatibility.
The remote branch's VS2026 vendored spdlog/fmt Debug failure is not resolved by
the VS2022 build. No claim is made for unexercised examples/resource variants.
