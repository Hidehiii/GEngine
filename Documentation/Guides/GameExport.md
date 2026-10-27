# Build and package a project

Open or create a `.gproject` through **File > New project / Open project**.
Choose **Project > Package project...**, select **one output directory**, enter an
executable name, and optionally choose a logo image. Click **Build and package**.
The editor compiles the engine and project in Release, converts the logo to an
embedded Windows icon, creates a resource archive, then publishes the result.
There is no prebuilt-EXE picker, packaging settings page, or separate preview
operation. Progress and compiler errors appear in Console; Cancel packaging stops
the owned build tree.

## Project contents

The complete version-1 descriptor is:

```json
{ "schemaVersion": 1, "name": "MyGame" }
```

It contains **no packaging information**: no executable, output, runtime flags
or packaging object. Legacy descriptors must remove their packaging object.
Source files live in `src/`, assets in `Assets/`, and optional config/notices in
`Config.ini` and `Licenses/` alongside the descriptor. New project creation
provides a minimal native `src/Game.cpp`. The included FrameGraphTriangle.gproject
is a complete source-project example.

Games use the normal GEngine CreateApplication contract and include
GEngine/EntryPoint.h, as the generated starter and example do. Custom unrelated
main functions/build systems require adaptation; they do not acquire packaged
startup merely by placing source files in this directory.

Asset references must resolve within that project-relative layout. Existing
absolute paths or external source-directory references are not rewritten; their
conversion and dependency discovery remain separate asset-pipeline work.

Output, executable name and logo are remembered only in local editor preferences
and never change the descriptor. The executable name defaults to the project name
and must be a valid Windows base name. Choose a destination outside the project.
Only empty folders or unmodified exporter-owned output can be replaced;
user-added or modified files cause a protective failure.

## Output and execution

The selected folder contains one root entry executable named after the packaging
executable name and category folders:

```text
MyGame.exe
Runtime/
  MyGame.exe
  GEngine.dll
  <required native runtime DLLs>
Data/
  MyGame.gpak
Metadata/
  .gengine-output.json
```

The root `MyGame.exe` is a small launcher with the optional logo embedded as its
Windows icon. It starts the real game executable from `Runtime/`, which keeps all
native DLL imports beside that executable and uses the same custom name.
Assets, config and licenses are inside `Data/MyGame.gpak`, with Assets/Cache
excluded. No loose Assets tree or PowerShell launcher is shipped. Start the
root EXE directly from any cwd. The runtime executable validates the adjacent
categorized pack, extracts it into a unique `%TEMP%/GEngineGames/<session>`
directory, then starts the application there. Config rewrites, caches and logs
cannot modify the published pack. Session directories are retained; persistent
settings/save migration and automatic cleanup remain open. Games overriding
WorkingDirectory must respect the packaged session instead of a developer path.

The initial format is **uncompressed and unencrypted**. It merges/checks files;
it does not convert textures/models/shaders or stream directly from the archive.
Missing required pack, unsupported version, checksum failure, unsafe/duplicate
paths and truncation fail before application construction. Checksums are integrity
checks, not authentication against someone who can rewrite a pack and its hashes.
Direct on-demand `.gpak` reads and package-external writable state are planned as
S5.2; until then the shipped entry point validates and extracts the complete pack
before application construction.
The S5.3 categorized layout is implemented: the root launcher keeps the user
visible entry point while the real executable and native DLLs remain together in
`Runtime/`. Publication metadata is not a payload file and lives under
`Metadata/`.

## Build and publication requirements

The editor needs CMake, Visual Studio C++ tools and a complete
GEngine source SDK/dependency checkout. It discovers the SDK above its deployed
location. A relocated standalone editor without that SDK cannot compile games.
These are automatic tool choices, not extra packaging inputs. Incremental build
caches live under `%TEMP%/GEngineBuilds/<project-id>`; compilation is always
invoked, although up-to-date sources may be skipped.

The first automatic pipeline builds native games with managed scripting excluded.
Requesting scripting from that engine explicitly fails; C# compilation/Mono
deployment is unfinished. General builds still default to Mono support;
`GE_WITH_MONO=OFF` / Premake `--without-mono` explicitly build without it.

Publication verifies sibling staging before replacing the selected folder.
Changed output retains the old folder as `<output>.previous-<id>`. Prepublication
failure preserves the old folder; failed directory switching attempts rollback.
Windows directory replacement is not claimed atomic: interruption in the switch
can require restoring the retained previous folder. History/user files are never
automatically deleted. Verified unchanged output uses zero payload copies.
Changed files copy from builds; unchanged output files may hard-link only from
prior immutable output, never mutable source/build products.

A Windows job owns the build tree, so forced cancellation stops its compiler
children. Project/output locks refuse competing operations. Directories are trusted
and must not be mutated concurrently by external processes. The editor uses a
thirty-minute deadline. System graphics drivers/runtimes and the Visual C++ runtime
are player prerequisites; CMake/Visual Studio are not.

Vendor DLL collection follows the existing build runtime set. Audit redistribution
rights/notices and clean-machine prerequisites before a public release; automatic
PE import and license discovery remain open.

## Command line and validation

```bat
dist\cmake-vs2022\windows-x86_64\Release\GEngineTools\GEngineTools.exe export-project --project Example/FrameGraphTriangle/FrameGraphTriangle.gproject --output E:\GameExports\Triangle --executable-name TriangleGame --icon C:\Icons\TriangleGame.ico
dist\qt-editor\windows-x86_64\Release\GEngine-EditorHost\GEngine-EditorHost.exe --packaging-self-test
```

The first command performs the editor operation; output is required, while the
executable name and `.ico` icon are optional deployment parameters. The editor
converts supported image logos to `.ico` before invoking the exporter. Host
`--packaging-self-test` exercises the output/name/logo dialog, invalid inputs,
custom manifest identity, embedded icon, real build, repeat, compile failure,
cancel/retry and metadata preservation with isolated local preferences. These
checks do not prove physical clean-machine installation,
long-run GPU memory stability or support for arbitrary third-party project builds.

Earlier directory-deployment prototypes and their PowerShell tests were removed;
the native `.gproject`/`.gpak` pipeline supersedes them.
