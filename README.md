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

## Usage

```sh
cbt help

Usage:

cbt <options> [file_name]

Options:

create-project <project-name>   - Create a new project

create-file <file_name>         - Generate header and respective C++ files under 'headers/' and 'src/' directories
create-file <path/to/file_name> - Same as above, but will create necessary sub-directories if required

compile-project                 - Compile all files and generate respective binaries under 'build/binaries/<respective-paths>'
clear-build                     - Delete all object files under 'build/' directory

info                            - Show information regarding cbt
help                            - Shows this help message
```
