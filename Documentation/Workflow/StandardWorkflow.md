# Standard workflow

This is the repository's documentation synchronization standard. Follow the
repository-wide engineering rules in [AGENTS.md](../../AGENTS.md) as well.

## Every code or plan update

1. Register the work in the appropriate `TODO/` file before changing code or
   plans. Record scope, acceptance criteria, and the current status.
2. Read the relevant architecture and public API descriptions alongside the
   code, headers, and examples before making the change.
3. After each code or plan update, update **both** the relevant
   [architecture function page](../Architecture/Architecture.md) and
   [API function page](../API/README.md) in the same work item. Use the indexes
   to select pages; update an index when topic coverage or file paths change.
   Describe affected ownership, data flow, backend boundaries, and public API
   or usage where applicable. Label proposed behavior as planned; do not
   present it as implemented until code exists. If one side has no behavioral
   change, record that explicitly instead of silently skipping it.
4. Keep other focused documents and example READMEs consistent with the
   selected architecture/API pages. Update the TODO entry with implementation
   status, verification
   evidence, and remaining work.
5. Review links and content, inspect the diff, and run `git diff --check` before
   handoff. A documentation-only update needs no engine rebuild; code changes
   follow the build and backend checks in [AGENTS.md](../../AGENTS.md).

Documentation is part of the change, not a later cleanup task. A plan entry
must remain visibly distinct from a supported API or verified implementation.
