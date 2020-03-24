#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

int main(int argc, char* argv[])
{
	std::ifstream input(argv[1]);
	std::filesystem::path outputPath(argv[2]);
	auto targetDir = outputPath.remove_filename();

	if (!std::filesystem::is_directory(targetDir))
    {
	    std::filesystem::create_directories(targetDir);
    }

	std::ofstream output(argv[2]);
	output << "R\"GLSL(\n";
	output << input.rdbuf();
	output << "\n)GLSL\"";
	
	std::cout << "Literified " << argv[1] << std::endl;
	std::cout << argv[2] << std::endl;
	return 0;
}