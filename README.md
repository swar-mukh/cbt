# cbt

`cbt` or "**C**++ **B**uild **T**ool" is a postmodern build tool intended towards developing applications in C++ effortlessly.

## Philosophy

The one thing lacking in the C++ ecosystem is a simple and lucid build system. While the flexibility of C++ and the low-level target nature of this language has enabled/forced multiple vendors to create their own (both platform-agnostic and platform-dependent) build systems, it is really difficult to interact with such tools due to one or more of the following (a non-enhaustive list) issues:

- Different syntax to learn
- Some such tools generate output which is then to be fed to some other tool which then builds your application, e.g. `Meson`, `Ninja`
- The language itself (owing to its development history) demanding understanding of intrinsic low-level or operating system specific features which then needs to be made portable
- Different tools having different folder layout structure adding to cognitive load
- Need to understand flow of commands in `Make` and family of build tools, e.g. `CMake`; and also continuously update the MakeFile (and variants) with cryptic glob patterns
- Some build tools being written in some other interpreted language requiring the end-developer to download extra runtime environments, e.g. `SCons`
- Some tools require knowledge of the GUI (**G**raphical **U**ser **I**nterface) IDE (**I**ntegrated **D**evelopment **E**nvironment) that interacts with the underlying build tool (e.g. Visual C++ Build Tool); while some CLI (**C**ommand **L**ine **I**nterface) tools are so complex that considerable effort has to be made to first understand cryptic flags and arguments (e.g. `clang`, `g++`)
- _And lot more ..._

One reason why proliferation of JavaScript has been so prominent is due to `npm` (later other package managers followed suite) and the one unified `package.json`, both of which have catapulted a once browser-only language to be used in almost all (sometimes even inappropriately) domains.

`cbt` is similar in spirit to `npm` and `package.json`, except it is more about making software development with C++ more approachable to beginners and seasoned developers alike; and equally not raising C++ to such higher abstraction language(s) like Java, C#, etc.

## Media

1. [Lightning Talk](https://www.youtube.com/watch?v=JWoAhr2TkSY) at CppIndiaCon-2024 ([cbt-2024.07.30](https://github.com/swar-mukh/cbt/releases/tag/cbt-2024.07.30) being demoed).

## Features

`cbt` is a very simple build tool made from first principles:

- Entirely command driven, e.g. `cbt create-application my_app`, `cbt create-library my_lib`, `cbt create-file some_module/some_util`
- Simple commands to compile, test and build the project
- Native support for **timestamp-aware compilation and testing** without any extra involvement/effort from developer's end
- Simple, intuitive and maintainable project configuration through `project.cfg`
- Unlike an `npm` project, ensure that all projects created with `cbt` strictly have the same directory structure
- Automatically create proper scaffold upon invocation of `cbt create-file <file_name>` (descriptions given below), and thereby reduce cognitive load
- First-class (and **type-safe**) support for `env` file(s)
- **Decentralised** dependency management via `cbt resolve-dependencies`
- Unobtrusive and transparent `cbt_tools` offered to bring quality-of-life improvements as well as enable flexible customisation(s) as per project requirements
- Interface with `C` files through dedicated `headers/c/` and `src/c` directory allowing you to code in `C` (when absolutely required) and seamlessly invoke from `C++` file
- First-class support for containerisation technologies (like `Docker` and `Podman`) allowing you to develop, test and ship applications/libraries from day one 
- Leverage `cppcheck` to perform static analysis through a simple `cppcheck{}` interface available in `project.cfg`

## Usage

```sh
$ cbt help

Usage:

1. Globally                     - cbt <options> [file_name]
2. From inside a project        - [env=<an_env_file>] cbt <options> [file_name]

Note:

1. Environment files are located under the `<project>/environments/` directory
2. The '.env.template' file must be provided with all the environment values along with the necessary types
3. `env` entry defaults to 'local.env' if no `env` entry is provided
4. Specify the `env` file to be picked up without the '.env' extension, e.g. "env=production"

Options:

create-application <name>       - Scaffold a new application
create-library <name>           - Scaffold a new library

create-file <file_name>         - Generate respective C++ files under 'headers/', 'src/' and 'tests/unit_tests/' directories
create-file <path/to/file_name> - Same as above, but will create necessary sub-directories if required

create-c-file <file_name>       - Generate respective C files under 'headers/c', 'src/c' and 'tests/unit_tests/c' directories (will create necessary sub-directories if required)

resolve-dependencies            - Sync dependencies through 'project.cfg'
resolve-dependencies-afresh     - Remove existing dependencies and re-sync through 'project.cfg'

compile-project                 - Compile all files and generate respective binaries under 'build/binaries/'
build-project                   - (For applications only) Perform linking and generate final executable under 'build/'
run-unit-tests                  - Run all test cases under 'tests/unit_tests/' directory

perform-static-analysis         - Invoke `cppcheck` with respective parameters from 'project.cfg'

clear-build                     - Delete all object files under 'build/' directory

info                            - Show information regarding cbt
help                            - Shows this help message
```

## Installation

Ready-made binaries for Ubuntu and Windows are available through [Releases](https://github.com/swar-mukh/cbt/releases).

For development through containerisation technologies (e.g. `docker` or `podman`), head over to the [Containerisation](#containerisation) section. For other platforms, or for building from source, read on.

### Notes

1. `cbt` is developed in `WSL2 Ubuntu 20.04` and `Windows 11`.
2. There is a hard dependency on the `C++20` standard.
3. Depending on your toolchain preference i.e. either `gcc` or `llvm`, ensure your toolchain is at least `11.4.0` for `GNU`, or at least `19.1.4` for `LLVM`.
4. If on Windows, MinGW can be downloaded from [WinLibs](https://winlibs.com/).
5. On a Mac, the built-in `clang` is officially **not** supported. Download `LLVM 19` explicitly.
6. Ensure `curl` and `tar` are available.
7. `cbt` relies on `cppcheck` to perform static analysis (through `cbt perform-static-analysis` command). Head over to the official [website](http://cppcheck.net/) and follow the instructions on how to download. If on Windows 10/11, `cppcheck` can be downloaded using `winget` by issuing `winget install cppcheck` (probable download location: `C:\Program Files\Cppcheck`) and adding it manually to your `PATH`.

### Steps

1. Clone/download the source code and navigate to the directory.
2. Default toolchain is `gcc` picked up from `project.cfg` or if no `TOOLCHAIN` environment variable is found. To use `clang++`, set `toolchain=llvm` in `project.cfg` or set `TOOLCHAIN` environment variable to `llvm`.
3. Run the following:
  ```sh
  # On *nix platforms
  $ chmod +x script.sh
  $ ./script.sh init compile build

  # On Windows
  > .\script.bat init compile build
  ```
4. The executable will be placed under the `build/` directory with the name `cbt` (or `cbt.exe` on `Windows`).
5. Run `./build/cbt help` (or `.\build\cbt.exe` on `Windows`) to get all available commands.
6. Add the binary to your OS' `PATH`:
  ```sh
  # On *nix platforms
  $ ./script.sh link

  # On Windows
  > .\script.bat link
  ```
7. Run `cbt help` or `cbt info` to ensure that it is available globally.

**Note:** During the `build` stage through `script.sh`, pay attention to the following:

1. If your platform is `Linux` but not `Ubuntu`, update the file-name at line 40.
2. If you get a message _"Please use a binary that can generate a SHA-256 checksum for this platform"_, then add your respective platform and associated binary that computes the `SHA-256` checksum.
3. If `uname` is not available on your platform, replace `$(uname -s)` with `"$OSTYPE"` in line 39, and update the options accordingly.

In any of the above case(s), the format for the file-name containing the checksum is `<platform>.sha256.checksum.txt`.

### Containerisation

If you are developing using `docker` or `podman`, use the following workflow:

1. Build the image:
  ```sh
  # Set `TOOLCHAIN` to either `gcc` (default) or `llvm`
  $ docker build --build-arg TOOLCHAIN=gcc -t cbt .
  ```
2. Mount the project directory into the container:
  ```sh
  # On *nix platforms
  $ docker run -it --rm -v "$(pwd):/cbt" cbt bash

  # On Windows (via Command Prompt)
  > docker run -it --rm -v %cd%:/cbt cbt bash
  
  # On Windows (via PowerShell)
  > docker run -it --rm -v ${PWD}:/cbt cbt bash
  
  # On Windows (via Git Bash)
  > docker run -it --rm -v "/$(pwd):/cbt" cbt bash

  # On Windows (via WSL2)
  # Ensure WSL Integration (Settings > Resources > WSL Integration) is ON,
  # and the respective distro is selected
  > docker run -it --rm -v "$(wslpath -w "$PWD"):/cbt" cbt bash
  ```
3. Whatever changes you make to the source code, will now be reflected in the container, allowing you to compile and test from within the container itself.

## Roadmap

**Disclaimer**: The following roadmap may be subjected to change depending on various factors like: bandwidth, priority and/or complexity.

| # | Task | Status | Notes |
|:-:|------|:------:|-------|
| 1 | Create a basic workable binary | `✅ Complete` ||
| 2 | Allow basic `create`, `compile` and `build` functionalities | `✅ Complete` ||
| 3 | Support scaffold for `test`ing | `✅ Complete` | _**Note:** Only unit tests supported currently_ |
| 4 | Use `project.cfg` as a one-stop configuration file for the entire project workspace management | `✅ Complete` |
| 5 | Decouple `g++` and all build configurations from source code | `✅ Complete` | _<ul><li>Requires completion of `project.cfg` parser first.</li><li>Need to add other strict and sane compiler flags.</li></ul>_ |
| 6 | Add support for various other C++ compilers | `✅ Complete` ||
| 7 | Support scaffold for shared objects (`.so` and `.dll`) | `💤 TBD` ||
| 8 | Add support for dependency management | `✅ Complete` | _<ul><li>Add support for **Software Bill of Materials** (**Update: Postponed for now**)</li></ul>_ |
| 9 | Add support for documentation during scaffold and as a command (maybe via some 3rd party tools like `doxygen`) | `💤 TBD` ||
| 10 | Bootstrap `cbt` with `cbt` | `✅ Complete` ||
| 11 | Add native support for reading environment values from `env` file(s) | `✅ Complete` ||
| 12 | Optimise compilation and building by inspecting dependency graph | `✅ Complete` | _<ul><li>Should we go all in with C++ modules instead? (**Update: Postponed for now**)</li><li>How to mix-n-match traditional HPP/CPP files with module files in the same project? (**Update: Postponed for now**)</li></ul>_ |
| 13 | Support scaffold for daemons (*nix) and services (Windows)  | `💤 TBD` | _Refer to [this](https://github.com/swar-mukh/cbt/discussions/2) discussion_|
| 14 | Add support for `lint`ing (especially static analysis) | `✅ Complete` | _`cppcheck` seems to be a good fit for integration_ |
| 15 | Add support for working with containerisation technologies  | `✅ Complete` ||
