#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char* argv[])
{
	std::ifstream input(argv[1]);

	std::ofstream output(argv[2]);
	output << "R\"GLSL(\n";
	output << input.rdbuf();
	output << "\n)GLSL\"";
	
	std::cout << "Literified " << argv[1] << std::endl;
	return 0;
}