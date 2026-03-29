#ifndef ANAN_STRING_HPP
#define ANAN_STRING_HPP
#include <stack>
#include <string>
#include <vector>
#include <stdint.h>
#include <cwctype>
#include <algorithm>
namespace ananStr{
    std::wstring fixBrackets(const std::wstring& input);
    //该函数将空格视为标点符号
    size_t GetFirstPunctuationPos(const std::wstring& text);

    void removeSpaces(const std::wstring& text, std::wstring& out);

    std::vector<std::wstring> splitByDelimiters(const std::wstring& text);
    std::vector<std::wstring> splitEvenly(const std::wstring& text, uint32_t count);
    size_t split(const std::wstring&text, wchar_t chr, std::vector<std::wstring>&out, size_t pos);
    void split(const std::wstring& text, std::vector<std::wstring>& out);
    std::vector<std::wstring> split(const std::wstring& text, uint32_t count);

    inline bool IsLineBreak(wchar_t c){
        return c == L' ' || c == L'[';
        // return c != '!' && c != '?' && c != '.' && c != '~' && (std::iswpunct(c) || c == L' ');
    }
};
#endif