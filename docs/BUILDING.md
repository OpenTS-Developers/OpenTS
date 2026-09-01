# Building OpenTS

> [!IMPORTANT]
> Visual Studio 2022 Win32 Debug and Release builds are supported. Both have
> been verified from a fresh CMake configuration. A successful build
> establishes compilation, not runtime behavior.

## Supported target

| Component | Requirement |
| --- | --- |
| Host and architecture | Windows, 32-bit (`Win32`) target |
| Processor | SSE2, so a Pentium 4 or Athlon 64 onward |
| Generator and compiler | Visual Studio 2022 MSVC 19.30 or newer |
| Windows SDK | A Visual Studio-installed Windows SDK |
| CMake | 3.23 or newer |
| C++ language level | C++20 |
| Configurations | Debug and Release |

Other generators, compilers, architectures, and configurations are not
supported by the current tree.

Install Visual Studio 2022 with the **Desktop development with C++** workload,
a Windows SDK, CMake 3.23 or newer, and Git for Windows.

## Dependencies

The renderer is built on [bgfx](https://github.com/bkaradzic/bgfx), vendored as the
`thirdparty/bgfx.cmake` submodule and pinned to a tested tag. It carries bgfx, bx, and
bimg as submodules of its own, so the checkout must be recursive:

```powershell
git submodule update --init --recursive
```

A fresh clone can do the same in one step with `git clone --recurse-submodules`.
Configuration fails with instructions if the submodule is missing. Updating the
dependency means moving the submodule to a new tag in its own change.

## Configure and build

Run these commands from the repository root in PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug
cmake --build build --config Release
```

CMake normally discovers Visual Studio through the Visual Studio Installer. If
the installation is not registered, provide its installation directory and
product version through `CMAKE_GENERATOR_INSTANCE`.

The generated solution exposes only Debug and Release. Successful builds write
the engine executable under `build/bin/<configuration>/` under its runtime
name and copy the runtime files into `TS_RUN_DIR`, which defaults to `Run/`:

| Configuration | Runtime files |
| --- | --- |
| Debug | `GameD.exe`, `GameD.pdb`, `GameD.map`, `Language.dll` |
| Release | `Game.exe`, `Game.pdb`, `Game.map`, `Language.dll` |

`Language.dll` has the same name in both configurations, so the most recently
built configuration replaces the previous copy in `Run/`. Compiler and linker
intermediates remain under the selected build directory.

## Experimental clang-cl cross-build

An unsupported Linux cross-build is available for compiler-portability work. It
uses native `clang-cl`, LLD, LLVM library and resource tools, and UASM with the
MSVC headers and libraries. It does not expand the supported build matrix or
establish runtime behavior.

The reconstructed codebase may still contain undefined behavior that the
supported MSVC build happens not to expose. A successful clang-cl build may
therefore run incorrectly or fail at runtime; validate any result separately.

Provide a directory containing a Visual Studio layout and Windows SDK. The
cross-build uses the layout's default MSVC toolset and newest complete SDK.
Configure a single-configuration Ninja build:

```bash
cmake -S . -B build/clang-cl -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-cl-msvc.cmake \
  -DOPENTS_MSVC_ROOT=/path/to/msvc
cmake --build build/clang-cl
```

The toolchain requires `clang-cl`, `lld-link`, `llvm-lib`, `llvm-mt`, `llvm-rc`,
and `uasm` on `PATH`. It exports `compile_commands.json`;
`.vscode/c_cpp_properties.clang.example.json` reads that file for IntelliSense.

## Build from Visual Studio Code

Visual Studio Code (VSCode) support includes (assuming recommended extensions are installed):

- pre-configured CMake Tools settings;
- build tasks corresponding to the configs (a configure task and a config
  picker, plus two hidden per-configuration ones backing the launch configs);
- launch and attach configs;
- full Test Explorer integration.

Standard VSCode shortcuts (`Ctrl+Shift+B`, `F5`, `Ctrl+F5`) and interface apply.

## Build identity

The project version is declared once, by `project(OpenTS VERSION ...)` in the
top-level `CMakeLists.txt`, with any SemVer prerelease label alongside it in
`OPENTS_VERSION_PRERELEASE`, because `project()` accepts numbers only. Both must
match the development entry of the manual's release registry, which
`python manual/tools/manage.py check` enforces.

Each build writes two generated headers from that version and the repository
state:

| Header | Contents |
| --- | --- |
| `opents_version.h` | The version components, the version string, a prerelease flag, and the packed version number |
| `opents_build.h` | The commit, branch, commit date, whether tracked files were modified, and the version as it is displayed |

The packed version number is the major, minor, and patch components in one byte
each. The save game stamp and the network version are that number, so different
release-cycle versions refuse one another. Development snapshots within one
cycle share the number; their saves, replays, and network sessions are not
promised to interoperate. A prerelease is not distinguished there and carries
the identity of the release it leads up to.

Everything that names a version to the player reads these headers: the version
resources of `Game.exe` and `Language.dll`, the title screen, the version
dialog, the crash report, and the debug log's opening banner. A build reports
its version with the commit it came from, as in `0.1.0 (ab12cd3)`, and adds a
modification marker when tracked files differ from that commit. The commit is a
diagnostic build identity, not an enforced save or network compatibility stamp.
Configuring with
`-DOPENTS_OFFICIAL_BUILD=ON` reports the version alone, for a build published
under the version it declares.

The version stamp is rewritten only when the version changes, so an ordinary
commit does not recompile the code that reads it. The build stamp refreshes on
every build, so committing is reflected without reconfiguring, and an unchanged
stamp is not rewritten.

A detached checkout, which is what building a tag or a pull request produces, has
no branch of its own. The stamp then reports a ref that points at the commit,
preferring a tag, so a continuous integration build of a pull request reports
that pull request rather than the bare word `HEAD`.

Git is not required. A build with no Git available, or from a source archive
with no repository, succeeds and reports the commit as `unknown` and the version
without one.

## Continuous integration

The `Engine` workflow builds every pull request that is ready for review and
every push to `main` that touches the engine, its build files, or the workflows
themselves. A draft pull request builds nothing until it is marked ready, which
starts the build for the commit it then carries. The `Engine nightly` workflow
builds on a daily schedule; when nothing has been committed since the last one,
the scheduled run cancels itself so that the latest successful nightly is
always one that produced artifacts, which keeps the nightly download links
resolvable. Both call the same reusable `Engine build` workflow, which on a
Windows runner with Visual Studio 2022 configures and builds Win32 Debug and
Release with the commands above, runs the CTest suite, and uploads each
configuration's executable, language library, and symbol file as an artifact
named for the configuration and the short commit. The linker map is not
uploaded, because the symbol file covers the same ground. After a successful
pull-request build, the `Engine build comment` workflow keeps one comment on
the pull request with direct nightly.link downloads of that build's artifacts.

The `Engine release` workflow runs when a GitHub release is published. It
builds the release's commit with `-DOPENTS_OFFICIAL_BUILD=ON`, packages
`Game.exe`, `Language.dll`, and `Game.pdb` into a zip named for the release
tag, attaches the zip to the release, and appends release notes generated from
the manual's change records by `python manual/tools/manage.py release-notes`.
[Maintaining](../manual/MAINTAINING.md) owns the release procedure around it.

Continuous integration builds redirect `TS_RUN_DIR` to an empty directory, so an
uploaded artifact holds only the files that build produced.

Continuous integration establishes the same thing a local build does, on the
runner's toolchain. It does not establish runtime behavior.

## Verification boundary

The supported matrix was verified on August 16, 2026 with CMake 4.3.3, Visual
Studio 2022 Community 17.14.37328.6, MSVC 19.44.35228, and Windows SDK
10.0.26100. Fresh Win32 Debug and Release builds completed successfully. The
builds retain inherited MSVC warnings; warnings are not treated as errors, but
contributions should not add new warnings.

Build verification establishes that the supported toolchain compiles and links
the configured targets and produces the listed artifacts. Runtime behavior is
established separately, by play testing, and is outside this build-support
record.

The repository contains no maps, movies, audio, or other original game assets.
Keep legally obtained runtime data local and outside version control. Do not
commit populated run directories, original executables, proprietary SDKs, IDE
state, compiler output, generated CMake projects, or credentials.
