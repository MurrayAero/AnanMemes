#ifndef COMMAN_PARAMETER_HPP
#define COMMAN_PARAMETER_HPP
#include <string>
#include <locale>
#include <string>
#include <cstdint>
#include <sstream>
#ifdef _WIN32
#include <Windows.h>
char *optarg;
int32_t optind = 1;
int32_t opterr;
int32_t optopt;
#define no_argument 0
#define required_argument 1
#define optional_argument 2
#else
#include <codecvt>
#include <unistd.h>
extern char *optarg;
extern int32_t optind = 1;
extern int32_t opterr;
extern int32_t optopt;
#endif
namespace cp{
    struct CommandParameter{
        float angle = 0;
        std::wstring text;
        uint32_t face = 0;
        std::string image = "";
        std::string out = "out.png";
        std::string ananImage = "";
        std::string current_path = "";
        glm::uvec3 textColor = glm::uvec3(1);
        std::string fontFile = FONT_PATH"PinRuShouXieTi-1.ttf";
    };
#ifdef _WIN32
    int32_t find_short_option(char c, const char* optstring) {
        if (!optstring) return -1;
        
        const char* pos = strchr(optstring, c);
        if (!pos) return -1;
        
        if (pos[1] == ':') {
            return required_argument;
        } else if (pos[1] == ':' && pos[2] == ':') {
            return optional_argument;
        }
        
        return no_argument;
    }
#endif
    int32_t getopt(int32_t ___argc, char *const *___argv, const char *__shortopts){
#ifdef _WIN32
        static int32_t next_char_index = 1;
        
        optarg = nullptr;
        
        if (optind >= ___argc || ___argv[optind] == nullptr) {
            return -1;
        }
        
        if (next_char_index == 1) {
            if (___argv[optind][0] != '-' || strcmp(___argv[optind], "-") == 0 || strcmp(___argv[optind], "--") == 0) {
                return -1;
            }
            if (strcmp(___argv[optind], "--") == 0) {
                optind++;
                return -1;
            }
        }
        
        char c = ___argv[optind][next_char_index];
        if (c == '\0') {
            optind++;
            next_char_index = 1;
            return getopt(___argc, ___argv, __shortopts);
        }
        
        next_char_index++;
        optopt = c;
        
        int has_arg = find_short_option(c, __shortopts);
        
        if (has_arg == -1) {
            if (opterr) {
                std::cerr << ___argv[0] << ": invalid option -- '" << c << "'" << std::endl;
            }
            return '?';
        }
        
        if (has_arg == required_argument || has_arg == optional_argument) {
            if (___argv[optind][next_char_index] != '\0') {
                optarg = &___argv[optind][next_char_index];
                optind++;
                next_char_index = 1;
            } else {
                if (optind + 1 >= ___argc) {
                    if (has_arg == required_argument) {
                        if (opterr) {
                            std::cerr << ___argv[0] << ": option requires an argument -- '" << c << "'" << std::endl;
                        }
                        return ':';
                    } else {
                        optarg = nullptr;
                        optind++;
                        next_char_index = 1;
                        return c;
                    }
                }
                
                optarg = ___argv[optind + 1];
                if (optarg[0] == '-' && has_arg == optional_argument) {
                    optarg = nullptr;
                    optind++;
                    next_char_index = 1;
                } else {
                    optind += 2;
                    next_char_index = 1;
                }
            }
        } else {
            if (___argv[optind][next_char_index] == '\0') {
                optind++;
                next_char_index = 1;
            }
        }
        
        return c;
#else
        return ::getopt(___argc, ___argv, __shortopts);
#endif
    }
    static inline std::string get_parameter_path(const std::string& path) {
        if (path.empty()) {
            return "";
        }
        size_t lastSlash = path.find_last_of("/\\");
        
        if (lastSlash == std::string::npos) {
            return "";
        }

        if (lastSlash == path.length() - 1) {
            return get_parameter_path(path.substr(0, path.length() - 1));
        }
        return path.substr(0, lastSlash  + 1);
    }
#ifdef _WIN32
    static inline std::wstring utf8_to_wstring(const char* str) {
        if (!str || !*str) return L"";
        int u8len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,str, -1, nullptr, 0);
        if (u8len > 0) {//str为utf-8
            std::vector<wchar_t> buf(u8len);
            MultiByteToWideChar(CP_UTF8, 0, str, -1, buf.data(), u8len);
            return std::wstring(buf.data());
        }
        //str为gbk
        int gblkLen = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
        if (gblkLen <= 0) return L"";
        std::vector<wchar_t> buf(gblkLen);
        MultiByteToWideChar(CP_ACP, 0, str, -1, buf.data(), gblkLen);
        return std::wstring(buf.data());
    }
#else
    static inline std::wstring utf8_to_wstring(const char* str) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        return cvt.from_bytes(str);
    }
#endif
    static inline void help(int32_t argc, char *argv[]){
        printf("usage:[-t text] [-i image] [-f face] [-o output image]\n");
        printf("example:-t\"input text\" -i input.png -f6 -o out.png\n");
        printf("option:\n");
        printf("\tbase:\n");
        printf("\t\t-t<text>\ttext on anan's notepad.\n");
        printf("\t\t-i<image>\timage on anan's notepad.\n");
        printf("\t\t-f<face>\tanan's face or specify font file\n");
        printf("\t\t-o<output image>\t\tanan's\n");
        printf("\toperate:\n");
        printf("\t\t-r<angle>\tangle;rotate anan's.\n");
        printf("\t\t-n<no add hand>\twill not add hand of anan's.\n");
        printf("\t\t-c<text color>\twill change text color;format:R,G,B. for explain:255,0,0\n");
        printf("\t\t-a<anan image>\twill change anan image\n");
        printf("face index:\n");
        printf("\t0 = happy\n");
        printf("\t1 = normal\n");
        printf("\t2 = yandere\n");
        printf("\t3 = blush\n");
        printf("\t4 = angry\n");
        printf("\t5 = speechless\n");
        printf("\t6 = tsundere\n");
        printf("\t7 = Nervous\n");
        printf("explain:\n");
        printf("\tuse -t, -i, or both.\n");
        printf("\tenglish delimiters only.\n");
    }
    static inline bool GetArgument(int32_t argc, char *argv[], CommandParameter *parameter){
//         int32_t wargc;
//         wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if(argc < 2){
            return false;
        }
        //getopt_long长选项--
        int32_t opt;
        parameter->current_path = get_parameter_path(argv[0]);
        if(parameter->current_path[0] == '~'){
            parameter->current_path = getenv("HOME") + parameter->current_path.substr(1);
        }
        while ((opt = getopt(argc, argv, "t:i:f:o:r:c:a:")) != -1){
            if(opt == 't'){
                parameter->text = utf8_to_wstring(optarg);
            }
            else if(opt == 'c'){
                std::stringstream ss(optarg);
                float r, g, b;
                char delim;  // 用于读取分隔符
                ss >> r >> delim >> g >> delim >> b;
                if (ss.fail()) {
                    parameter->textColor = glm::vec3(rand() % MAX_BYTE, rand() % MAX_BYTE, rand() % MAX_BYTE);
                }
                else{
                    parameter->textColor = glm::vec3(r, g, b);
                }
            }
            else if(opt == 'r'){
                parameter->angle = atoi(optarg);
            }
            else if(opt == 'a'){
                parameter->ananImage = optarg;
            }
            else if(opt == 'i'){
                parameter->image = optarg;
            }
            else if(opt == 'f'){
                const uint32_t faceIndex = atoi(optarg);
                if(strlen(optarg) > 5){
                    parameter->fontFile = optarg;
                }
                else{
                    parameter->face = faceIndex;
                }
            }
            else if(opt == 'o'){
                const char *png = strstr(optarg, ".png"), *jpg = strstr(optarg, ".jpg");
                if(png || jpg){
                    parameter->out = optarg;
                }
            }
        }
        return true;
    }
};
#endif