#include <cstring>
#include <iostream>
#include "ananString.hpp"
#include "AnansMems.hpp"
#include "command_parameter.hpp"
bool prepare_command_parameter(int32_t argc, char *argv[], cp::CommandParameter *parameter){
    if(!cp::GetCommandParameter(argc, argv, parameter)){
        cp::help(argc, argv);
        return false;
    }
    if(parameter->text == L"" && parameter->image == "" && parameter->face < (uint32_t)AnansFace::MiniHappy){
        parameter->text = L"你忘记添加文本了";
    }
    parameter->text = ananStr::fixBrackets(parameter->text);
    if(parameter->face > (uint32_t)AnansFace::MiniBase){
        printf("invalid face index(%d), max face index is %d. used random face\n", parameter->face, AnansFace::MiniBase);
        parameter->face = rand() % ((uint32_t)AnansFace::MiniBase + 1);
    }
    return true;
}
int main(int32_t argc, char *argv[]){
    cp::CommandParameter parameter;
#ifndef DEBUG
#ifdef _WIN32
    srand(time(nullptr));
    setlocale(LC_ALL, "en_US.UTF-8");
#else
    srandom(time(nullptr));
#endif // _WIN32
    prepare_command_parameter(argc, argv, &parameter);
#endif
    AnansMemes anan(parameter.current_path);
    //TOFO:写一个自动获取记笔记偏移和大小的功能
    glm::uvec2 offset, extent;
#ifdef DEBUG
    parameter.face = (uint32_t)AnansFace::Tsundere;
    // anan.AddText(L"给吾辈[点赞投币]", offset, extent);
    // anan.AddText(L"AB[CD]EFG[HIJK]", offset, extent);
    // anan.AddText(L"给吾辈[闭嘴]", offset, extent);
    // anan.AddText(L"吾辈 劝你归顺[中国]", offset, extent);
    // anan.AddText(L"吾辈 劝你归顺[中华人民 共和国]", offset, extent);
    // parameter.text = L"比较好看的夏目安安";
    parameter.text = L"让我们的新安安看新算法的效果";
    // parameter.image = "out.png";
    // parameter.text = L"hello word";
#endif
    if(parameter.face == (uint32_t)AnansFace::Tsundere){
        offset = glm::uvec2(80, 550);
        extent = glm::uvec2(650, 350); 
    }
    else if(parameter.face < (uint32_t)AnansFace::Tsundere){
        offset = glm::uvec2(90, 430);
        extent = glm::uvec2(335, 205);
    }
    else if(parameter.face == (uint32_t)AnansFace::MiniBase){
        offset = glm::uvec2(80, 135);
        extent = glm::uvec2(150, 90);
    }
    else{
        offset = glm::uvec2(183, 495);
        extent = glm::uvec2(235, 110);
    }
    anan.SetFace((AnansFace)parameter.face);
    if(parameter.image != "" && parameter.face < (uint32_t)AnansFace::MiniHappy){
        anan.AddImage(parameter.image, offset, extent);
    }
    if(parameter.text != L""){
        anan.AddText(parameter.text, offset, extent);
    }
    if(parameter.face < (uint32_t)AnansFace::MiniHappy){
        anan.AddHand();
    }
    anan.SaveImage(parameter.out);
}
