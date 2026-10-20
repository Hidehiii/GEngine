# Render graph

[API index](README.md) · [Rendering architecture](../Architecture/Rendering.md)

`RenderGraph` is the backend-neutral scheduler for a frame. It owns pass
dependencies, resource versions, per-resource access declarations, and the
resulting state transitions. It does not own application resources; imported
resources remain owned by their texture, buffer, or presentation system.

## Topic index

- [Basic pattern and versions](RenderGraph/BasicPattern.md)
- [Execution and attachments](RenderGraph/Execution.md)
- [Resources and states](RenderGraph/Resources.md)
- [Transient resources](RenderGraph/TransientResources.md)
- [Presentation integration](RenderGraph/Presentation.md)

## Basic pattern

Use explicit resource versions when dependencies must be independent of pass
creation order. Each write extends the latest version. Reads depend on that
version's producer; the next writer waits for readers because versions share
physical storage. Branching writes and mixed versioned/legacy accesses are
rejected.
