## Presentation and lifecycle follow-up (2026-10-11)

- [ ] Resolve the Vulkan validation diagnostics exposed by SDK 1.4.363.0.
  Scope: request only debug-messenger message types supported by the enabled
  instance extensions, stop enabling deprecated Vulkan device layers, and fix
  presentation semaphore reuse (per-swapchain-image semaphores or the
  documented swapchain-maintenance path). Acceptance: a Debug
  FrameGraphTriangle Vulkan run with validation enabled completes 120 frames,
  passes all three frame-119 readbacks, exits normally, and logs no validation
  errors or warnings. Resize/minimize behavior remains covered by the separate
  interactive lifecycle item. Reproduction: the installed validation layer
  reports the unsupported `VK_EXT_device_address_binding_report` messenger
  type, nonzero `VkDeviceCreateInfo.enabledLayerCount`, and signal semaphores
  still in use by the swapchain.
- [ ] Verify FrameGraphTriangle's visible triangle and color/motion changes on
  each available backend, then resize and minimize/restore the live window and
  close it normally. Acceptance: no crash or validation/debug error; rendering
  resumes at the new size and after restoration; the frame-119 compute and
  array-layer pixel readbacks pass. Record each backend and any UI limitation
  separately. Preserve the user's local Config.ini selection. The user asked
  for no desktop control, so this interactive portion remains unverified.
- [x] Run a bounded multi-minute replacement workload on each available
  backend, sample private bytes after warm-up, and compare with replacement
  disabled where useful. Acceptance: report sample times, backend/debug mode,
  normal exit and numerical readbacks, and any sustained growth without
  claiming indefinite leak freedom. Keep unrun backend cases open.
  On 2026-10-11, the existing VS2022 Debug executable ran each case for 120
  seconds without desktop input. Each exited 0 and passed both frame-119
  readbacks; stderr was empty. OpenGL used the user's selected API, while
  D3D12 and Vulkan used isolated configuration copies. Replacement was on
  except in the stated control. An earlier direct launch outside the example
  working directory aborted during startup; all measured runs used the
  documented working directory. Samples are private bytes, not GPU allocations:

  | Backend / mode | Frames | 10 s | 60 s | 120 s |
  | --- | ---: | ---: | ---: | ---: |
  | OpenGL 4.6 | 396350 | 173027328 | 172883968 | 172888064 |
  | D3D12 12.2, debug layer on | 60272 | 192450560 | 200187904 | 210235392 |
  | D3D12 12.2, debug layer off | 125314 | 170655744 | 170479616 | 171003904 |
  | D3D12 12.2, debug layer on, replacement off | 62511 | 192475136 | 202747904 | 211525632 |
  | Vulkan 1.4.312, dedicated queues | 76092 | 414896128 | 414720000 | 419622912 |

  Vulkan selected graphics=0, compute=2, transfer=5; no validation messages
  appeared. D3D12's debug layer logged one existing warning that an initial
  buffer UAV state is ignored in favor of COMMON, but no error. The Vulkan
  late increase and D3D12 debug-layer growth need more diagnosis; these
  bounded samples do not prove leak freedom or identify memory ownership.
  A normal shutdown is covered, but visible output, real resize, minimize,
  and restore are not. Release and Vulkan shared-queue long runs were not
  repeated in this milestone.
