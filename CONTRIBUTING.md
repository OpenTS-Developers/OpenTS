# Contributing to OpenTS

OpenTS welcomes focused bug reports, proposals, documentation changes, and
pull requests. Visual Studio 2022 Win32 Debug and Release are the supported
development targets. A successful build is not runtime evidence.

## Before starting

Review time is limited. Search existing issues and pull requests, then discuss
any non-trivial change with a maintainer and get agreement on its scope before
implementation. This includes features, intentional behavior changes,
subsystem replacements, and compatibility breaks. Use `#dev-chatter` or
`#dev-forum` for early discussion and a GitHub issue to record decisions.
Unsolicited non-trivial pull requests may wait a long time for review or be
closed.

Read [Building OpenTS](docs/BUILDING.md) and [Style](docs/STYLE.md) before
changing source. Never commit game assets, original executables, proprietary
SDKs, credentials, personal data, IDE state, or build output.

## Current priorities

Work toward the first [development milestone](README.md#state-and-plans) has
review priority. Portability planning and preparatory work may proceed in
parallel. Other pull requests are deprioritized and may wait for review;
simple bug fixes are reviewed as time permits.

## Pull request workflow

Public contributions go through pull requests; maintainers may commit
directly when appropriate. Pull requests are squash-merged, so keep each one
focused and give it a summary suitable for project history. Keep mechanical
cleanup separate from behavior changes: formatting, renaming, or ownership
work must not hide changes to gameplay, formats, persistence, or networking.

Classify externally visible behavior as one of the following:

- **Preserved:** only the implementation changes. Explain why existing
  documentation remains accurate.
- **Fixed:** the change corrects behavior that conflicts with the project's
  goals.
- **Intentionally changed:** the project deliberately chooses a different
  result, such as a feature, balance or performance change, or removal.

A player- or modder-visible fix or intentional change needs a manual change
record, categorized as a feature, fix, balance, performance, or internal
change. Include migration steps for compatibility breaks. See
[Documentation](#documentation).

The TibSun reconstruction and original executable are historical evidence.
They do not define correctness or acceptance for active OpenTS development.

## Compatibility boundaries

Treat configuration, data formats and defaults, saves, replays, networking,
deterministic simulation, COM interfaces, and layout-sensitive structures as
compatibility boundaries. Changes there can break mods, maps, saved state,
network games, or external consumers without an obvious build failure.

Before changing a boundary:

1. Record the current behavior and its evidence.
2. Identify affected versions, data, mods, saved state, peers, and consumers.
3. Add focused tests or other reproducible evidence.
4. Update the owning documentation in the same pull request.
5. Give practical migration steps for an incompatible change.

Compatibility across versions exists only where documented. Saves and network
sessions carry the project version, so different releases cannot exchange
saves or play together. Development snapshots in one release cycle share that
version stamp even when their saves, replays, network protocol, or simulation
differ. Do not assume snapshots work together. Test the current snapshot,
describe the release impact, and open a new development version only through
the release process in
[Maintaining](manual/MAINTAINING.md). [Build identity](docs/BUILDING.md#build-identity)
explains the version and diagnostic commit identifiers.

## Source changes

- Use C++20 for new or substantially rewritten C++, while modernizing inherited
  code incrementally.
- Keep the incremental move toward an entity-component architecture possible;
  [Project direction](docs/DIRECTION.md) explains the constraints.
- Follow nearby naming and layout, and format only touched code.
- Keep honest reconstruction placeholders until evidence supports a better
  name.
- Follow [Style](docs/STYLE.md) for comments and historical or legal notices.

## Documentation

Every pull request must say whether documentation needs changing. Update the
relevant guide for changes to behavior, interfaces, configuration, commands,
scripting, compatibility, architecture, builds, or contributor workflow. If a
mechanical, test-only, or internal change needs no update, say why the existing
text remains accurate.

Player- or modder-visible engine changes must update the
[OpenTS manual](manual/README.md) and its matching lifecycle record. Put each
fact in one guide and link to it elsewhere. Document current behavior,
supported inputs, relevant limits, and migration requirements; do not present
plans or assumptions as current behavior.

For manual work, follow [Authoring](manual/AUTHORING.md) and
[Manual style](manual/STYLE.md). Changes to manual tooling, schemas, generated
data contracts, lifecycle machinery, routes, or publishing also require
[Maintaining](manual/MAINTAINING.md).

## AI-assisted contributions

You are responsible for everything you submit, whether or not AI helped
produce it. Use AI only for work you understand well enough to design, verify,
explain, and maintain yourself. Check generated code and prose against the
source or observed behavior, then edit it to project style, especially comments
and documentation. AI output is not evidence and receives no commit
attribution.

If you could not own and maintain a contribution without AI, do not submit it.
Ask a developer or maintainer for a bounded task that is easy to check, such as
researching, drafting, or testing a proposed fix or workaround. Agree on the
scope first.

Keep issues, reviews, and pull-request discussion concise and natural. A short
human explanation is better than generated boilerplate.

## Validation

Run the narrowest relevant check first. Report exact commands,
configurations, environments, and results, together with relevant checks not
run. Do not describe configure or build success as runtime testing.

Build the affected supported configuration for source changes. Build both
Debug and Release when changing shared build setup, compiler-dependent code,
or behavior that optimization may affect. Existing MSVC warnings remain;
identify new warnings instead of describing the build as warning-free.

Behavior changes need focused, reproducible evidence. Automated tests must not
require proprietary game assets or original executables. CI builds Win32 Debug
and Release and runs CTest for ready engine pull requests; draft pull requests
do not run these checks until marked ready. This is build evidence and does not
replace any runtime testing the change needs.
[Building OpenTS](docs/BUILDING.md#continuous-integration) documents the
workflow.

Every pull request touching `code/` also needs the manual change record
described above. Purely mechanical work with no player- or modder-visible
effect can use the `no change record` label; applying it re-runs the check.

## Pull request content

Include:

- a concise summary and rationale;
- the behavior classification and affected compatibility boundaries;
- exact validation results and relevant checks not run;
- documentation changes, or why none are needed; and
- screenshots or recordings when they provide useful evidence.

Contributions are submitted under [the repository license](LICENSE.md),
including its applicable additional terms.
