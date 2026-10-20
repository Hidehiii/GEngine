## Extension points

- Put cross-backend concepts in `GEngine/include/GEngine` and portable
  orchestration in `GEngine/src/GEngine`.
- Add backend implementations only for native behavior; do not expose native
  handles through application/renderer interfaces.
- Declare resource access/state through `RenderGraph` rather than a layer's
  backend-specific barriers.
