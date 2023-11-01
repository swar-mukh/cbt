#include <iostream>

int main(const int argc, char *argv[], char *envp[]) {
	std::cout << "Argument count: " << argc << std::endl;

	while (*envp) {
		std::cout << "Env.: " << *envp++ << std::endl;
	}

	for (int i{0}; i < argc; ++i) {
		std::cout << "Argument " << i + 1 << ": " << argv[i] << std::endl;
	}

	return EXIT_SUCCESS;
}
