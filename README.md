# cbt

`cbt` or "**C**++ **B**uild **T**ool" is a single binary build tool aimed to provide a standard application structure and facilitate code re-use, thereby making working with C++ a breeze.

## Philosophy

The one thing lacking in the C++ ecosystem is a simple and lucid build system. While the flexibility of C++ and the low-level target nature of this language has enabled/forced multiple vendors to create their own (both platform-agnostic and platform-dependent) build systems, it is really difficult to interact with such tools due to one or more of the following (a non-enhaustive list) issues:

- Different syntax to learn
- Some such tools generate output which is then to be fed to some other tool which then builds your application
- The language itself (owing to its development history) demanding understanding of intrinsic low-level or operating system specific features which then needs to be made portable
- Different tools having different folder layout structure adding to cognitive load
- Need to understand flow of commands in `Make` and family of build tools
- Some build tools being written in some other interpreted language requiring the end-developer to download extra runtime environments
- Some tools require knowledge of the [GUI](## "Graphical User Interface") [IDE](## "Integrated Development Environment") that interacts with the underlying build tool; while some [CLI](## "Command Line Interface") tools are so complex that considerable effort has to be made to first understand cryptic flags and arguments
- _And lot more ..._

The reason why proliferation of JavaScript has been so prominent is due to `npm` (later other package managers followed suite) and the one unified `package.json`, both of which have catapulted a once browser-only language to be used in almost all (sometimes even inappropriately) domains.

`cbt` is similar in spirit to `npm` and `package.json`, except it is more about making software development with C++ more approachable to beginners and seasoned developers alike; and equally not raising C++ to such higher abstraction language(s) like Java, C#, etc.

## Features

`cbt` is a very simple build tool made from first principles:

- Entirely command driven, e.g. `cbt create-project my-app`, `cbt create-file some_module/some_util`
- Simple commands to build and test the project
- Unlike an `npm` project, ensure that all projects created with `cbt` strictly have the same folder structure
- Automatically create proper scaffold upon invocation of `cbt create-file <file_name>` (descriptions given below), and thereby reduce cognitive load
- First-class (and **type-safe**) support for `env` file(s)

## Usage

```sh
$ cbt help

Usage:

[env=<an_env_file>] cbt <options> [file_name]

Note:

1. Environment files are located under the `environments/` directory
2. The '.env.template' file must be provided with all the environment values along with the necessary types
3. `env` entry defaults to 'local.env' if no `env` entry is provided
4. Specify the `env` file to be picked up without the '.env' extension, e.g. "env=production"

Options:

create-project <project-name>   - Create a new project

create-file <file_name>         - Generate header and respective C++ files under 'headers/' and 'src/' directories
create-file <path/to/file_name> - Same as above, but will create necessary sub-directories if required

compile-project                 - Compile all files and generate respective binaries under 'build/binaries/<respective-paths>'
clear-build                     - Delete all object files under 'build/' directory

build-project                   - Perform linking and generate final executable under 'build/' (requires project compilation first)

info                            - Show information regarding cbt
help                            - Shows this help message
```

## Installation

### Notes

1. Currently, `cbt` is exclusively developed in `WSL2 Ubuntu 20.04`. But MinGW for Windows should also do the trick. Future support for other platforms is planned.
2. There is a hard dependency on `g++` as the underlying tool.
3. There is a hard dependency on `C++2a` standard. Ensure your `gcc` toolchain is at least `9.4.0`.

### Steps

1. Clone/download the source code and navigate to the directory
2. Run the following:
  ```sh
  $ mkdir -p build/binaries/workspace
  $ chmod +x script.sh
  $ ./script.sh
  ```
3. The executable will be placed under the `build/` directory with the name `cbt`
4. Run `./build/cbt help` to get all available commands

## Roadmap

**Disclaimer**: The following roadmap may be subjected to change depending on various factors like: bandwidth, priority and/or complexity.

| # | Task | Status | Notes |
|:-:|------|:------:|-------|
| 1 | Create a basic workable binary | `✅ Complete` ||
| 2 | Allow basic `create`, `compile` and `build` functionalities | `✅ Complete` ||
| 3 | Support scaffold for `test`ing | `⏳ In Progress` ||
| 4 | Use `project.cfg` as a one-stop configuration file for the entire project workspace management | `⏳ In Progress` |
| 5 | Decouple `g++` and all build configurations from source code | `💤 TBD` | _<ul><li>Requires completion of `project.cfg` parser first.</li><li>Need to add other strict and sane compiler flags.</li></ul>_ |
| 6 | Add support for various other C++ compilers | `💤 TBD` | _Requires a slight forward thinking mindset w.r.t. point 8 below_ |
| 7 | Support scaffold for shared objects (`.so` and `.dll`) | `💤 TBD` ||
| 8 | Add support for dependency management | `💤 TBD` | _<ul><li>How to handle dependencies developed with another compiler implementation than current project?</li><li>Scaffold should support generating library code along with executable code during project creation.</li></ul>_ |
| 9 | Add support for documentation during scaffold and as a command (maybe via some 3rd party tools like `doxygen`) | `💤 TBD` ||
| 10 | Bootstrap `cbt` with `cbt` | `⛅ Partially achieved` ||
| 11 | Add native support for reading environment values from `env` file(s) | `✅ Complete` | _Need to update the documentation regarding the method of invocation for the application instead of `cbt`_ |
| 12 | Optimise compilation and building by inspecting dependency graph | `💤 TBD` | _<ul><li>Should we go all in with C++ modules instead?</li><li>How to mix-n-match traditional HPP/CPP files with module files in the same project?</li></ul>_ |
| 13 | Support scaffold for daemons (*nix) and services (Windows)  | `💤 TBD` ||
