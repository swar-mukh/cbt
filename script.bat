@echo off
setlocal

set CPP_STANDARD=c++2a
set SAFETY_FLAGS=-Wall -Wextra -pedantic

set HEADERS_DIR=headers
set BUILD_DIR=build
set BINARIES_DIR=%BUILD_DIR%/binaries

set COMPILE_FLAGS=-std=%CPP_STANDARD% %SAFETY_FLAGS% -Os -s -c -I%HEADERS_DIR%/
set BUILD_FLAGS=-std=%CPP_STANDARD% %SAFETY_FLAGS% -O3 -s

echo.

for %%a in (%*) do (
    if %%a == init (
        call :init
    ) else if %%a == compile (
        call :compile
    ) else if %%a == build (
        call :build
    ) else if %%a == clean (
        call :clean
    ) else (
        echo Invalid option!
        exit /b 1
    )
    echo.
)

exit /b 0

:init
    echo "==========="
    echo "Phase: init"
    echo "==========="
    echo.
    echo "[CREATE] 'build/' directory"
    mkdir build\binaries\workspace
    mkdir build\test_binaries\unit_tests
    exit /b 0

:compile
    echo "=============="
    echo "Phase: compile"
    echo "=============="
    echo.
    echo "[COMPILE] src/gnu_toolchain.cpp" && g++ %COMPILE_FLAGS% src/gnu_toolchain.cpp -o %BINARIES_DIR%/gnu_toolchain.o
    echo "[COMPILE] src/workspace/dependencies_manager.cpp" && g++ %COMPILE_FLAGS% src/workspace/dependencies_manager.cpp -o %BINARIES_DIR%/workspace/dependencies_manager.o
    echo "[COMPILE] src/workspace/env_manager.cpp" && g++ %COMPILE_FLAGS% src/workspace/env_manager.cpp -o %BINARIES_DIR%/workspace/env_manager.o
    echo "[COMPILE] src/workspace/modification_identifier.cpp" && g++ %COMPILE_FLAGS% src/workspace/modification_identifier.cpp -o %BINARIES_DIR%/workspace/modification_identifier.o
    echo "[COMPILE] src/workspace/project_config.cpp" && g++ %COMPILE_FLAGS% src/workspace/project_config.cpp -o %BINARIES_DIR%/workspace/project_config.o
    echo "[COMPILE] src/workspace/scaffold.cpp" && g++ %COMPILE_FLAGS% src/workspace/scaffold.cpp -o %BINARIES_DIR%/workspace/scaffold.o
    echo "[COMPILE] src/workspace/util.cpp" && g++ %COMPILE_FLAGS% src/workspace/util.cpp -o %BINARIES_DIR%/workspace/util.o
    echo "[COMPILE] src/commands.cpp" && g++ %COMPILE_FLAGS% src/commands.cpp -o %BINARIES_DIR%/commands.o
    echo "[COMPILE] src/main.cpp" && g++ %COMPILE_FLAGS% src/main.cpp -o %BINARIES_DIR%/main.o
    exit /b 0

:build
    echo "============"
    echo "Phase: build"
    echo "============"
    echo.
    echo "[BUILD] build/cbt.exe" && g++ %BUILD_FLAGS% %BINARIES_DIR%/*.o %BINARIES_DIR%/workspace/*.o -o build/cbt.exe
    echo "[HASH] build/cbt.exe" && certutil -hashfile build\cbt.exe SHA256 > build\Windows.sha256.checksum.txt
    exit /b 0

:clean
    echo "============"
    echo "Phase: clean"
    echo "============"
    echo.
    rmdir /s /q build
    echo "[DELETE] sample application (if present)" && rmdir /s /q my_application
    echo "[DELETE] sample library (if present)" && rmdir /s /q my_library
    echo.
    call :init
    exit /b 0
