readonly CPP_STANDARD="c++2a"
readonly SAFETY_FLAGS="-Wall -Wextra -pedantic"

readonly HEADERS_DIR="headers"
readonly BUILD_DIR="build"
readonly BINARIES_DIR="$BUILD_DIR/binaries"

readonly COMPILE_FLAGS="-std=$CPP_STANDARD $SAFETY_FLAGS -Os -s -c -I$HEADERS_DIR/"
readonly BUILD_FLAGS="-std=$CPP_STANDARD $SAFETY_FLAGS -O3 -s"

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
    echo "[COMPILE] src/workspace/env_manager.cpp" && g++ $COMPILE_FLAGS src/workspace/env_manager.cpp -o $BINARIES_DIR/workspace/env_manager.o
    echo "[COMPILE] src/workspace/modification_identifier.cpp" && g++ $COMPILE_FLAGS src/workspace/modification_identifier.cpp -o $BINARIES_DIR/workspace/modification_identifier.o
    echo "[COMPILE] src/workspace/project_config.cpp" && g++ $COMPILE_FLAGS src/workspace/project_config.cpp -o $BINARIES_DIR/workspace/project_config.o
    echo "[COMPILE] src/workspace/scaffold.cpp" && g++ $COMPILE_FLAGS src/workspace/scaffold.cpp -o $BINARIES_DIR/workspace/scaffold.o
    echo "[COMPILE] src/workspace/util.cpp" && g++ $COMPILE_FLAGS src/workspace/util.cpp -o $BINARIES_DIR/workspace/util.o
    echo "[COMPILE] src/commands.cpp" && g++ $COMPILE_FLAGS src/commands.cpp -o $BINARIES_DIR/commands.o
    echo "[COMPILE] src/main.cpp" && g++ $COMPILE_FLAGS src/main.cpp -o $BINARIES_DIR/main.o
}

function build() {
    echo "============"
    echo "Phase: build"
    echo "============"
    echo
    echo "[BUILD] build/cbt" && g++ $BUILD_FLAGS $BINARIES_DIR/*.o $BINARIES_DIR/**/*.o -o build/cbt
    case $(uname -s) in
        Linux)  echo "[HASH] build/cbt" && sha256sum build/cbt > build/Ubuntu.sha256.checksum.txt;;
        Darwin) echo "[HASH] build/cbt" && shasum -a 256 build/cbt > build/MacOS.sha256.checksum.txt;;
        *)      echo "Please use a binary that can generate a SHA-256 checksum for this platform";;
    esac
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
        clean)   clean;;
        *)
            echo "Invalid option!"
            echo
            exit -1
            ;;
    esac
    echo
done
