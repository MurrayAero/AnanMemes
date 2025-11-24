#ifndef COMMAN_PARAMETER_HPP
#define COMMAN_PARAMETER_HPP
#include <locale>
#include <string>
#include <cstring>
#include <cstdint>
#include <codecvt>
#ifdef _WIN32
#include <Windows.h>
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
        printf("usage:%s 文本 [-i图片名称] [-f表情索引] [-o <图片名称>]\n", argv[0]);
        printf("\t表情索引:\n\t\t0 = 高兴\n\t\t1 = 平常\n\t\t2 = 病娇\n\t\t3 = 脸红\n\t\t4 = 愤怒\n\t\t5 = 无语\n\t\t6 = 高兴_q版\n\t\t7 = 傲娇_q版\n\t\t8 = 平常_q版\n\t\t9 = 傲娇_2_q版\n");
        printf("注意:\n");
        printf("\t[]必须用英文符号\n\t输出的文件只支持jpg、png图片\n");
        printf("\t有概率会发生文字重叠, 可以通过多次生成来重新调整位置\n");
        printf("\t如果多次调整仍有文字重叠, 则建议减少字数\n");
        printf("\t使用q版时,字数不能过多,特别是平常_q版, 目前仅容纳两字, 后续可能增多\n");
    }
    static inline bool GetCommandParameter(int32_t argc, char *argv[], CommandParameter *parameter){
        if(argc < 2){
            return false;
        }
#ifdef _WIN32
        int32_t wargc;
        wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        parameter->text = wargv[1];
#else
        parameter->text = utf8_to_wstring(argv[1]);
#endif
        parameter->current_path = get_parameter_path(argv[0]);
        for (int32_t i = 0; i < argc; i++){
            if(argv[i] && argv[i][0] == '-'){
                if(argv[i][1] == 'o'){
                    parameter->out = argv[i + 1];
                }
                else if(argv[i][1] == 'f'){
                    parameter->face = atoi(argv[i] + 2);
                }
                else if(argv[i][1] == 'i'){
                    parameter->image = argv[i] + 2;
                }
            }
            else{
                const char *png = strstr(argv[i], ".png"), *jpg = strstr(argv[i], ".jpg");
                if(png || jpg){
                    parameter->out = argv[i];
                }
            }
        }
        return true;
    }
};
#endif