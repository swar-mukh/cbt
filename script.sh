echo

echo "Deleting sample project (if present)" && rm -rf my-project

echo

echo "Compiling src/util/scaffold.cpp" && g++ -Wall -Wextra -pedantic -c -I ./headers/ src/util/scaffold.cpp -o ./build/binaries/util/text_content.o
echo "Compiling src/commands.cpp" && g++ -Wall -Wextra -pedantic -c -I ./headers/ src/commands.cpp -o ./build/binaries/commands.o
echo "Compiling src/main.cpp" && g++ -Wall -Wextra -pedantic -c -I ./headers/ src/main.cpp -o ./build/binaries/main.o

echo

echo "Building build/cbt" && g++ -O3 build/binaries/*.o build/binaries/**/*.o -lstdc++fs -o build/cbt

echo