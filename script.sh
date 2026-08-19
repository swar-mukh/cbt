readonly TOOLCHAIN="${TOOLCHAIN:-gcc}"

if [ "$TOOLCHAIN" = "gcc" ]; then
    _COMPILER="${CXX:-g++}"
    _CXXFLAGS="${CXXFLAGS:-}"
    _LDFLAGS="${LDFLAGS:-}"
else
    _COMPILER="${CXX:-clang++}"
    _CXXFLAGS="${CXXFLAGS:--stdlib=libc++ -fexperimental-library -D_LIBCPP_ENABLE_EXPERIMENTAL}"
    _LDFLAGS="${LDFLAGS:--stdlib=libc++ -rtlib=compiler-rt -fuse-ld=lld -lc++ -lc++abi}"
fi

readonly CXXFLAGS="$_CXXFLAGS"
readonly LDFLAGS="$_LDFLAGS"

readonly COMPILER="$_COMPILER"
readonly CPP_STANDARD="c++20"
readonly SAFETY_FLAGS="-Wall -Wextra -pedantic"

readonly HEADERS_DIR="headers"
readonly BUILD_DIR="build"
readonly BINARIES_DIR="$BUILD_DIR/binaries"

readonly COMPILE_FLAGS="-std=$CPP_STANDARD $SAFETY_FLAGS $CXXFLAGS -Os -s -c -I$HEADERS_DIR/"
readonly BUILD_FLAGS="-std=$CPP_STANDARD $SAFETY_FLAGS $CXXFLAGS $LDFLAGS -O3 -s"

function init() {
    echo "==========="
    echo "Phase: init"
    echo "==========="
    echo
    echo "[CREATE] 'build/' directory"
    mkdir -p build/{binaries/workspace,test_binaries/unit_tests}
}

function compile() {
    echo "=============="
    echo "Phase: compile"
    echo "=============="
    echo
    echo "[COMPILE] src/gnu_toolchain.cpp" && $COMPILER $COMPILE_FLAGS src/gnu_toolchain.cpp -o $BINARIES_DIR/gnu_toolchain.o
    echo "[COMPILE] src/workspace/dependencies_manager.cpp" && $COMPILER $COMPILE_FLAGS src/workspace/dependencies_manager.cpp -o $BINARIES_DIR/workspace/dependencies_manager.o
    echo "[COMPILE] src/workspace/modification_identifier.cpp" && $COMPILER $COMPILE_FLAGS src/workspace/modification_identifier.cpp -o $BINARIES_DIR/workspace/modification_identifier.o
    echo "[COMPILE] src/workspace/project_config.cpp" && $COMPILER $COMPILE_FLAGS src/workspace/project_config.cpp -o $BINARIES_DIR/workspace/project_config.o
    echo "[COMPILE] src/workspace/scaffold.cpp" && $COMPILER $COMPILE_FLAGS src/workspace/scaffold.cpp -o $BINARIES_DIR/workspace/scaffold.o
    echo "[COMPILE] src/workspace/util.cpp" && $COMPILER $COMPILE_FLAGS src/workspace/util.cpp -o $BINARIES_DIR/workspace/util.o
    echo "[COMPILE] src/commands.cpp" && $COMPILER $COMPILE_FLAGS src/commands.cpp -o $BINARIES_DIR/commands.o
    echo "[COMPILE] src/main.cpp" && $COMPILER $COMPILE_FLAGS src/main.cpp -o $BINARIES_DIR/main.o
}

function build() {
    echo "============"
    echo "Phase: build"
    echo "============"
    echo
    echo "[BUILD] build/cbt" && $COMPILER $BUILD_FLAGS $BINARIES_DIR/*.o $BINARIES_DIR/**/*.o -o build/cbt
    case $(uname -s) in
        Linux)  echo "[HASH] build/cbt" && sha256sum build/cbt > build/Ubuntu.sha256.checksum.txt;;
        Darwin) echo "[HASH] build/cbt" && shasum -a 256 build/cbt > build/MacOS.sha256.checksum.txt;;
        *)      echo "Please use a binary that can generate a SHA-256 checksum for this platform";;
    esac
}

function link() {
    echo "==========="
    echo "Phase: link"
    echo "==========="
    echo

    if [ ! -f build/cbt ]; then
        echo "[ACTION] Compile and build project first"
        exit 1
    fi

    if [ ! -d /opt/cbt ]; then
        sudo mkdir -p /opt/cbt
        echo "[DIR] Create '/opt/cbt/'"
    fi

    sudo cp build/cbt /opt/cbt/
    echo "[COPY] 'cbt' updated"

    if [[ ":$PATH:" != *":/opt/cbt:"* ]]; then
        CURRENT_SHELL=$(basename "$SHELL")

        case "$CURRENT_SHELL" in
            bash)     RC_FILE="$HOME/.bashrc";;
            zsh)      RC_FILE="$HOME/.zshrc";;
            ksh)      RC_FILE="$HOME/.kshrc";;
            tcsh)     RC_FILE="$HOME/.tcshrc";;
            csh)      RC_FILE="$HOME/.cshrc";;
            fish)     RC_FILE="$HOME/.config/fish/config.fish";;
            dash|ash) RC_FILE="$HOME/.profile";;
            *)        RC_FILE="$HOME/.profile";;
        esac

        if [ "$CURRENT_SHELL" = "fish" ]; then
            if ! grep -Fxq "set -Ux PATH /opt/cbt \$PATH" "$RC_FILE" 2>/dev/null; then
                echo "set -Ux PATH /opt/cbt \$PATH" >> "$RC_FILE"
            fi
        else
            if ! grep -Fxq "export PATH=\$PATH:/opt/cbt" "$RC_FILE" 2>/dev/null; then
                echo "export PATH=\$PATH:/opt/cbt" >> "$RC_FILE"
            fi
        fi

        echo "[ACTION] Restart your shell or run 'source $RC_FILE' to apply changes."
    else
        echo "[INFO] Updated 'cbt' available globally"
    fi
}

function clean() {
    echo "============"
    echo "Phase: clean"
    echo "============"
    echo
    rm -rf build
    echo "[DELETE] sample application (if present)" && rm -rf my_application
    echo "[DELETE] sample library (if present)" && rm -rf my_library
    echo
    init
}

echo

for i in "$@"; do
    case $i in
        init)    init;;
        compile) compile;;
        build)   build;;
        link)    link;;
        clean)   clean;;
        *)
            echo "Invalid option! Available options: 'init', 'compile', 'build', 'link' and 'clean'."
            echo
            exit -1
            ;;
    esac
    echo
done
