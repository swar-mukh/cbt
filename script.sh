readonly CPP_STANDARD="c++2a"
readonly SAFETY_FLAGS="-Wall -Wextra -pedantic"

readonly HEADERS_DIR="headers"
readonly BUILD_DIR="build"
readonly BINARIES_DIR="$BUILD_DIR/binaries"

readonly COMPILE_FLAGS="-std=$CPP_STANDARD $SAFETY_FLAGS -Os -s -c -I$HEADERS_DIR/"
readonly BUILD_FLAGS="-std=$CPP_STANDARD $SAFETY_FLAGS -O3 -Os -s"

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
    echo "[HASH] build/cbt" && sha256sum build/cbt > build/Ubuntu.sha256.checksum.txt
}

function clean() {
    echo "============"
    echo "Phase: clean"
    echo "============"
    echo
    rm -rf build
    echo "[DELETE] sample project (if present)" && rm -rf my-project
    echo
    init
}

echo

for i in "$@"; do
    case $i in
        init)
            init
            ;;
        compile)
            compile
            ;;
        build)
            build
            ;;
        clean)
            clean
            ;;
        *)
            echo "Invalid option!"
            echo
            exit -1
            ;;
    esac
    echo
done
