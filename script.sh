echo

echo "Deleting sample project (if present)" && rm -rf my-project

echo

echo "Compiling src/workspace/scaffold.cpp" && g++ -std=c++17 -Wall -Wextra -pedantic -c -I ./headers/ src/workspace/scaffold.cpp -o ./build/binaries/workspace/scaffold.o
echo "Compiling src/commands.cpp" && g++ -std=c++17 -Wall -Wextra -pedantic -c -I ./headers/ src/commands.cpp -o ./build/binaries/commands.o
echo "Compiling src/main.cpp" && g++ -std=c++17 -Wall -Wextra -pedantic -c -I ./headers/ src/main.cpp -o ./build/binaries/main.o

echo

echo "Building build/cbt" && g++ -std=c++17 -Wall -Wextra -pedantic -O3 -Os -s build/binaries/*.o build/binaries/**/*.o -o build/cbt

echo