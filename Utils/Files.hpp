#pragma once
#include <string>
#ifdef EMSCRIPTEN
#include "emscripten.h"
#include <stdlib.h>
#else
#include <string>
#include <fstream>
#include <streambuf>
#endif

namespace Files
{
#ifdef EMSCRIPTEN
    namespace
    {
        EM_JS(const char*, ReadTextJS, (const char* filePath), {
            var fs = require('fs');
            var jsString = fs.readFileSync(UTF8ToString(filePath), {encoding:'utf8', flag:'r'});
            var lengthBytes = lengthBytesUTF8(jsString)+1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
            return stringOnWasmHeap;
        });
    }

    std::string ReadText(const char* filePath)
    {
        const char* str = ReadTextJS(filePath);
        std::string text(str);
        free((void*)str);
        return text;
    }

    EM_JS(void, WriteText, (const char* filePath, const char* text), {
        var fs = require('fs');
        fs.writeFileSync(UTF8ToString(filePath), UTF8ToString(text));
    });

    EM_JS(void, CreateFolderIfNotExist, (const char* dirPath), {
        var fs = require('fs');
        var dir = UTF8ToString(dirPath);
        if (!fs.existsSync(dir)) {
            fs.mkdirSync(dir, {recursive: true});
        }
    });
#else
    std::string ReadText(const char* filePath)
    {
        std::ifstream fstr(filePath);
        return {std::istreambuf_iterator<char>(fstr), std::istreambuf_iterator<char>()};
    }

    void WriteText(const char* filePath, const char* text)
    {
        std::ofstream output(filePath);
        output << text;
    }

    void CreateFolderIfNotExist(const char* dirPath)
    {
        if (!std::filesystem::is_directory(dirPath))
        {
            std::filesystem::create_directories(dirPath);
        }
    }
#endif
}

