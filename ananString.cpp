#include "ananString.hpp"
    void ananStr::removeSpaces(const std::wstring& text, std::wstring& out) {
        out = text;
        out.erase(std::remove(out.begin(), out.end(), L' '), out.end());
    }
    std::wstring ananStr::fixBrackets(const std::wstring& input) {
        std::size_t pos = 0;
        std::wstring result = input;
        while(pos < result.length()){
            pos = result.find('[', pos);
            if(pos == std::wstring::npos)break;
            std::size_t symbol = GetFirstPunctuationPos(result.c_str() + pos + 1);
            if (symbol + pos + 1 != std::wstring::npos) {
                result.insert(symbol + pos + 1, L"]");
            } else {
                result.append(L"]");
            }
            pos += symbol + 1;
        }
        return result;
    }
    size_t ananStr::GetFirstPunctuationPos(const std::wstring& text) {
        for (size_t i = 0; i < text.length(); ++i) {
            if (IsLineBreak(text[i])) {
                return i;
            }
        }
        return std::wstring::npos;
    }

    std::vector<std::wstring> ananStr::splitByDelimiters(const std::wstring& text) {
        std::vector<std::wstring> result;
        std::wstring current_segment;
        
        for (wchar_t c : text) {
            if (std::iswpunct(c) || c == L' ') {
                if (!current_segment.empty()) {
                    result.push_back(current_segment);
                    current_segment.clear();
                }
            } else {
                current_segment += c;
            }
        }
        
        if (!current_segment.empty()) {
            result.push_back(current_segment);
        }
        
        return result;
    }
    std::vector<std::wstring> ananStr::splitEvenly(const std::wstring& text, uint32_t count) {
        std::vector<std::wstring> result;
        
        if (text.empty() || count == 0) {
            return result;
        }
        
        if (count == 1) {
            result.push_back(text);
            return result;
        }
        
        size_t text_length = text.length();
        size_t approx_chars_per_line = text_length / count;
        size_t remainder = text_length % count;
        
        size_t start_pos = 0;
        
        for (uint32_t i = 0; i < count; ++i) {
            if (start_pos >= text_length) {
                result.push_back(L"");
                continue;
            }
            
            size_t current_chars = approx_chars_per_line;
            if (i < remainder) {
                current_chars++;
            }
            
            size_t end_pos = start_pos + current_chars;
            if (end_pos > text_length) {
                end_pos = text_length;
            }
            
            result.push_back(text.substr(start_pos, end_pos - start_pos));
            start_pos = end_pos;
        }
        
        return result;
    }
    size_t ananStr::split(const std::wstring&text, wchar_t chr, std::vector<std::wstring>&out, size_t pos){
        const uint32_t len = text.length();
        size_t openBracket = text.find(chr, pos);
        if (openBracket == std::wstring::npos) {
            if (pos < len) {
                out.push_back(text.substr(pos));
            }
            return len;
        }
        if (openBracket > pos) {
            out.push_back(text.substr(pos, openBracket - pos));
        }
        return openBracket;
    }
    void ananStr::split(const std::wstring& text, std::vector<std::wstring>& out) {
        size_t pos = 0;
        while (pos < text.length()) {
            size_t closeBracket = split(text, ']', out, split(text, '[', out, pos));
            out[out.size() - 1] += L"]";
            pos = closeBracket + 1;
        }
    }
    std::vector<std::wstring> ananStr::split(const std::wstring& text, uint32_t count) {
        auto pos = GetFirstPunctuationPos(text);
        if (pos == std::wstring::npos) {
            return splitEvenly(text, count);
        }

        std::vector<std::wstring> result, temp;
        if(text.find('[') != std::wstring::npos){
            split(text, temp);
        }
        else{
            return splitByDelimiters(text);
        }
        for (auto&it:temp){
            auto out = splitByDelimiters(it);
            if(!out.empty()){
                if(it[0] == '['){
                    const uint32_t last = out.size() - 1;
                    out[0] = L"[" + out[0];
                    out[last] += L"]";
                }
                result.insert(result.end(), out.begin(), out.end());
            }
        }
        if(result.empty())result.push_back(text);
        return result;
    }