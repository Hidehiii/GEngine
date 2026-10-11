# Scene and physics follow-ups

- [ ] Wire or explicitly scope out 3D scene simulation. Current
  `Scene::OnAwake` creates `Physics3DWorld`, but `Scene::OnFixedUpdate` only
  calls `Physics2DWorld::Simulate`; no 3D step occurs through `Scene::OnUpdate`.
  Acceptance if supported: a scene-owned 3D rigid body advances under a fixed
  timestep in a focused example, with PhysX initialization and shutdown owned
  by the runtime. Otherwise document the 3D world as a manual/experimental API.
  This records a discovered limitation; no physics code change is in scope for
  the documentation audit.
