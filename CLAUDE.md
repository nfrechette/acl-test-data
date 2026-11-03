# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This repository holds the regression test data for the
[Animation Compression Library](https://github.com/nfrechette/acl) (ACL). It has two parts:

- `regression_tests/`: the reference clips (`*.acl.sjson`). A clip is a raw transform clip or a
  scalar track list. Most clips come from a subset of the CMU database; others are custom clips that
  test one feature or one edge case. `regression_tests/README.md` lists each clip and why it is there.
- `acl-sjson`: a C++11 tool that converts a clip from one ACL file format version to another, and
  between the text format (`.acl.sjson`) and the binary format (`.acl`).

ACL uses the packaged result: `make.py -package` creates `acl_regression_tests_vN.zip`, and the ACL
regression tests download it into `test_data/`.

## Build & convert

Everything goes through `make.py` (wraps CMake). Run `git submodule update --init --recursive`
first. The submodules are the ACL versions in `external/acl_v20`, `external/acl_v21`, and
`external/acl_v22` (each with its own RTM and sjson-cpp). CMake files go to `./build`, and the
installed tool goes to `./bin/acl-sjson`.

```bash
python3 make.py                      # generate project files only
python3 make.py -build               # build and install bin/acl-sjson (Release by default)
python3 make.py -clean -build        # clean rebuild
python3 make.py -convert -input ./regression_tests -output ./output_clips             # convert to the latest version
python3 make.py -convert -input ./regression_tests -output ./output_clips -target 2.1 # convert to a specified version
python3 make.py -package             # convert regression_tests/ and write acl_regression_tests_vN.zip
./bin/acl-sjson --info <file>        # print information about one clip
```

Useful variants: `-compiler {vs2019,clang14,gcc11,osx}`, `-cpu {x64,arm64}`, `-config {Debug,Release}`.
`-convert` with a directory input converts each clip to the binary format, and it removes the output
directory first. It skips `format_reference.acl.sjson`. `-package` uses `./output_regression_tests`
as output and copies `regression_tests/README.md` into the zip. `REGRESSION_TEST_DATA_VERSION` at the
top of `make.py` sets `N`; increase it when you release a new package.

There are no unit tests. CI (`.github/workflows/build.yml`) only builds on Linux, OS X, and Windows.
To verify a change, build, then run `-convert` (or `-package`) on `regression_tests/` and make sure
that each clip converts.

Builds use `-Wall -Wextra -Werror` (GCC/Clang) and `/W4 /WX` (MSVC) -- warnings break the build.
The tool defines `ACL_ON_ASSERT_ABORT` and `RTM_ON_ASSERT_ABORT`, so an ACL assert stops the conversion.

## Architecture & conventions

- **Layout**: `acl-sjson-core/` holds a representation of a clip that does not depend on an ACL
  version (`track_array`, `track`, `track_description`, `sample`, `metadata`, `acl_version`).
  Each `acl-vX.Y-shim/` is a static library that compiles against one ACL version and gives
  `acl_sjson_vXY::read_tracks` and `acl_sjson_vXY::write_tracks`. `acl-sjson/` is the executable
  (`main.cpp`, command line options, `convert`, `info`).
- **Several ACL versions in one binary**: each shim defines `ACL_ENABLE_VERSION_NAMESPACE`,
  `RTM_ENABLE_VERSION_NAMESPACE`, and `SJSON_CPP_ENABLE_VERSION_NAMESPACE`. Thus the symbols of
  each ACL version stay separate. Only a shim can include ACL headers. The core and the tool use only
  the `acl_sjson` types.
- **Read with the latest, write with the target**: `read_tracks` in `acl-sjson/sources/utils.cpp`
  always reads with the latest shim, because the latest ACL reads all older file formats.
  `convert.cpp` writes with the shim of the `--target` version. When there is no target, it keeps
  the source version.
- **Add an ACL version**: add the `external/acl_vXY` submodule (and its branch in `.gitmodules`).
  Copy the previous shim to `acl-vX.Y-shim/` and rename its namespace and target. Add the value to
  `acl_version` (and `to_string`), and move `latest` to it. Add the `--target` value in
  `command_line_options.cpp` and the cases in `convert.cpp`. Change `utils.cpp` to read with the
  new shim. Add the shim to the root `CMakeLists.txt` and to `acl-sjson/CMakeLists.txt`. Update the
  version lists in `README.md` and `external/README.md`.
- **The clip file format**: `regression_tests/format_reference.acl.sjson` documents each field of
  the `.acl.sjson` format and the history of the `version` value. It is not a valid clip. When a
  new ACL version changes the file format, update this file with the new version and fields.
- **Add a clip**: put the `.acl.sjson` file in `regression_tests/`. Add a line to the
  "Miscellaneous" section of `regression_tests/README.md` that tells how the clip is different
  from its source clip. Make sure that `make.py -convert` converts it with each target version
  that can represent it. `regression_tests/metadata.sjson` is not in the package (the ACL
  `make.py` writes its own), and it does not list all clips.
- **Header skeleton**: every header starts with `#pragma once` and the MIT license banner. Each
  source file has the same banner. Code is C++11 and uses tabs (see `.editorconfig`).
- **Generated files**: `build/`, `bin/`, `output_clips/`, `output_regression_tests/`, and the
  `acl_regression_tests_v*.zip` files are generated and ignored by git. Do not edit them.

## Communication style

Write in ASD-STE100 (Simplified Technical English). This applies to what you say to the user, to
commit messages, to pull requests, and to comments and documents in the tree. Identifiers in code keep their names;
the specification controls prose only.

- **One word, one meaning, one part of speech.** Do not use a noun as a verb. Write "measure the
  function", not "benchmark the function".
- **Use the approved word when there is one.** Write "start", not "kick off"; "remove", not "strip
  out"; "let", not "permit". The technical nouns of this tree -- track, sample, clip, segment,
  bit rate, quantization, range reduction -- are technical names and are approved by that rule.
- **Active voice, and name the agent.** Write "the function sets the bit rate", not "the bit rate
  is set". Use the passive voice only when the agent is not known or is not important.
- **Short sentences.** Maximum 20 words in an instruction, 25 in a description. Write one
  instruction in one sentence.
- **Do not use the -ing form as a noun or as an adjective.** Write "the code that loads", not
  "the loading code"; write "to clear the track", not "clearing the track".
- **Keep the articles and the helper words.** Write "the error on the output track", not "error on
  output track". Text that drops them is not faster to read.
- **Present tense, no figures of speech, and no words that carry an opinion.** A number is
  "5% lower", not "disappointing".
- **No Unicode characters** in comments, string literals, documents, commit messages, or pull
  requests. Use only ASCII. For example, use `<->` instead of `↔`, `--` instead of `—`, `->`
  instead of `→`. STE does not permit a Unicode dash either.

## Commit messages and pull requests

Commits and pull requests use the
[Angular commit message format](https://github.com/angular/angular/blob/main/CONTRIBUTING.md#commit).
No tool enforces the format, so check each message yourself. The main branch is `main`.

```
<type>(<scope>): <summary>

<body>

<footer>
```

- **Header** (required): `<type>(<scope>): <summary>`. The scope is optional, and most commits in
  this repository do not have one.
  - `type` is one of: `build`, `ci`, `docs`, `feat`, `fix`, `perf`, `refactor`, `test`. This repo
    also uses `chore` (for example `chore(ci): ...` or `chore: bump current test data version`) and
    `style` (for a change with no effect on behavior, such as a format change).
  - `scope` names the area of the change. Scopes in the history include `ci` and `tests`.
  - `summary` uses the imperative, present tense ("add", not "added" or "adds"). Do not capitalize
    the first letter. Do not put a period at the end.
- **Body**: use the imperative, present tense. Explain the motivation for the change. Compare the
  new behavior with the previous behavior when that helps.
- **Footer**: put breaking changes (`BREAKING CHANGE: <description>`), deprecations
  (`DEPRECATED: <description>`), and issue references (`Fixes #<number>`) here.

A pull request is merged with a merge commit. The pull request title must also use the
`<type>(<scope>): <summary>` format, and each commit in the pull request must use it too. The pull
request description follows the body rules above.
