#ifndef COMMAN_PARAMETER_HPP
#define COMMAN_PARAMETER_HPP
#include <locale>
#include <string>
#include <cstring>
#include <cstdint>
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
        std::wstring text;
        uint32_t face = 0;
        std::string image = "";
        std::string out = "out.png";
        std::string current_path = "";
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
        printf("usage:[-t text] [-i image] [-f face] [-o out image]\n");
        printf("example:-t \"input text\" -i input.png -f 6 -o out.png\n");
        printf("option:\n");
        printf("\t-t\ttext on anan's notepad.\n");
        printf("\t-i\timage on anan's notepad.\n");
        printf("\t-f\tanan's face\n");
        printf("\t-o\tanan's\n");
        printf("face index:\n");
        printf("\t0 = happy\n");
        printf("\t1 = normal\n");
        printf("\t2 = yandere\n");
        printf("\t3 = blush\n");
        printf("\t4 = angry\n");
        printf("\t5 = speechless\n");
        printf("\t6 = tsundere\n");
        printf("\t7 = happy_mini\n");
        printf("\t8 = tsundere_mini\n");
        printf("\t9 = normal_mini\n");
        printf("explain:\n");
        printf("\tuse -t, -i, or both.\n");
        printf("\tenglish delimiters only.\n");
    }
    static inline bool GetCommandParameter(int32_t argc, char *argv[], CommandParameter *parameter){
//         int32_t wargc;
//         wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if(argc < 2){
            return false;
        }
        //getopt_long长选项--
        int32_t opt;
        parameter->current_path = get_parameter_path(argv[0]);
        while ((opt = getopt(argc, argv, "t:i:f:o:")) != -1){
            if(opt == 't'){
                parameter->text = utf8_to_wstring(optarg);
            }
            else if(opt == 'i'){
                parameter->image = optarg;
            }
            else if(opt == 'f'){
                parameter->face = atoi(optarg);
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