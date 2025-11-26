#ifndef COMMAN_PARAMETER_HPP
#define COMMAN_PARAMETER_HPP
#include <locale>
#include <string>
#include <cstring>
#include <cstdint>
#include <codecvt>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
extern char *optarg;
#endif
namespace cp{
    struct CommandParameter{
        std::wstring text;
        uint32_t face = 0;
        std::string image = "";
        std::string out = "out.png";
        std::string current_path = "";
    };
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
#ifndef _WIN32
    static inline std::wstring utf8_to_wstring(const char* str) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
        return myconv.from_bytes(str);
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
#ifdef _WIN32
    static inline bool GetCommandParameter(int32_t argc, char *argv[], CommandParameter *parameter){
        if(argc < 2){
            return false;
        }
        int32_t wargc;
        wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

        parameter->current_path = get_parameter_path(argv[0]);
        for (int32_t i = 0; i < argc; i++){
            if(argv[i] && argv[i][0] == '-'){
                if(argv[i][1] == 'o' && argv[i + 1]){
                    parameter->out = argv[i + 1];
                }
                else if(argv[i][1] == 't' && argv[i + 1]){
                    parameter->text = wargv[i + 1];
                }
                else if(argv[i][1] == 'f' && argv[i + 1]){
                    parameter->face = atoi(argv[i + 1]);
                }
                else if(argv[i][1] == 'i' && argv[i + 1]){
                const char *png = strstr(argv[i + 1], ".png"), *jpg = strstr(argv[i + 1], ".jpg");
                    if(png || jpg){
                        parameter->image = argv[i + 1];
                    }
                }
            }
        }
        return true;
    }
#else
    static inline bool GetCommandParameter(int32_t argc, char *argv[], CommandParameter *parameter){
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
            else if(opt == 'i'){
                const char *png = strstr(optarg, ".png"), *jpg = strstr(optarg, ".jpg");
                if(png || jpg){
                    parameter->out = optarg;
                }
            }
        }
        return true;
    }
#endif
};
#endif