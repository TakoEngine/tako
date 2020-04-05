#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "Files.hpp"

int main(int argc, char* argv[])
{
    std::cout << argv[1] << std::endl;
    auto input = Files::ReadText(argv[1]);
    std::filesystem::path outputPath(argv[2]);
    auto targetDir = outputPath.remove_filename();
    Files::CreateFolderIfNotExist(targetDir.c_str());

#ifdef EMSCRIPTEN
    std::string header = "#version 100\nprecision mediump float;\n";
#else
    std::string header = "#version 120\n";
#endif
    Files::WriteText(argv[2], ("R\"GLSL(\n"  + header + input + "\n)GLSL\"").c_str());
	std::cout << "Literified " << argv[1] << std::endl;
	return 0;
}