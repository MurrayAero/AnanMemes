#include <cstring>
#include <iostream>
#include "ananString.hpp"
#include "AnansMems.hpp"
#include "command_parameter.hpp"
bool prepare_command_parameter(int32_t argc, char *argv[], cp::CommandParameter *parameter){
    if(!cp::GetArgument(argc, argv, parameter)){
        cp::help(argc, argv);
        return false;
    }
    if(parameter->text == L"" && parameter->image == ""){
        parameter->text = L"你忘记 添加文本了";
    }
    parameter->text = ananStr::fixBrackets(parameter->text);
    if(parameter->face > (uint32_t)AnansFace::Nervous){
        printf("invalid face index(%d), max face index is %d. used random face\n", parameter->face, AnansFace::Nervous);
        parameter->face = rand() % ((uint32_t)AnansFace::Nervous + 1);
    }
    return true;
}
//百分比, 但要给小数
glm::uvec4 calcNopepadSize(const std::string&ananImage, const glm::vec2&notepadOffsetScale, const glm::vec2&notepadSizeScale){
    int32_t width, height, c;
    char imageName[MAX_PATH];
    sprintf(imageName, "%s", ananImage.c_str());
    stbi_uc *data = stbi_load(imageName, &width, &height, &c, STBI_rgb_alpha);
    if(!data){
        printf("in function %s:failed to calculation nopepad size, anan image:%s\n", __FUNCTION__, ananImage.c_str());
        return glm::uvec4(0);
    }
    const glm::uvec2 offset = glm::vec2(width, height) * notepadOffsetScale, exent = glm::vec2(width, height) * notepadSizeScale;
    stbi_image_free(data);
    return glm::uvec4(offset, exent);
}
int32_t main(int32_t argc, char *argv[]){
    bool addHand = true;
    cp::CommandParameter parameter;
#ifndef DEBUG
#ifdef _WIN32
    srand(time(nullptr));
    setlocale(LC_ALL, "en_US.UTF-8");
#else
    srandom(time(nullptr));
#endif // _WIN32
    if(!prepare_command_parameter(argc, argv, &parameter)){
        return -1;
    }
#endif
    AnansMemes anan(parameter.current_path);
    //TODO:写一个自动获取记笔记偏移和大小的功能
    glm::uvec2 offset, extent;
#ifdef DEBUG
    // parameter.face = (uint32_t)AnansFace::Tsundere;
    // parameter.text = L"老婆们";
    // parameter.face = 7;
    // parameter.text = L"很晚了 大家晚安";
    // parameter.text = L"看垃圾 的眼神";
    // parameter.textColor = glm::uvec3(255, 0, 0);
    // parameter.text = L"给吾辈[点赞投币]";
    // parameter.face = (uint32_t)AnansFace::Nervous;
    // parameter.text = L"补全功能[是,否[正常?";
    // parameter.text = ananStr::fixBrackets(parameter.text);
    // parameter.text =L"给吾辈 [倒杯茶]";
    parameter.text =L"给吾辈 【闭嘴】";
    // parameter.text =L"给吾辈 [闭嘴]";
    // parameter.text = L"吾辈[劝你]归顺[中国]吧";
    // parameter.text = L"吾劝你归顺[中华人民共和国]";
    // parameter.text = L"比较好看 的夏目安安";
    // parameter.image = "out.png";
    // parameter.text = L"hello word";
#endif
    std::string ananImage;
    if(parameter.ananImage != ""){
        addHand = false;
        ananImage = parameter.current_path + parameter.ananImage;
        anan.SetAnanImage(ananImage);
    }
    else{
        ananImage = parameter.current_path + IMAGE_PATH + anan.GetAnansImageName((AnansFace)parameter.face);
    }
    if(parameter.face == (uint32_t)AnansFace::Tsundere){
        auto result = calcNopepadSize(ananImage, glm::vec2(0.112, 0.49), glm::vec2(.775, .3025));
        offset = glm::uvec2(result.x, result.y);
        extent = glm::uvec2(result.z, result.w);
    }
    else if(parameter.face == (uint32_t)AnansFace::Nervous){
        auto result = calcNopepadSize(ananImage, glm::vec2(0.15, 0.49), glm::vec2(.7, .35));
        offset = glm::uvec2(result.x, result.y);
        extent = glm::uvec2(result.z, result.w);
    }
    else{
        auto result = calcNopepadSize(ananImage, glm::vec2(0.175, 0.68), glm::vec2(.6, .30));
        offset = glm::uvec2(result.x, result.y);
        extent = glm::uvec2(result.z, result.w);
    }
    if(extent == glm::uvec2(0)){
        return -1;
    }
    anan.SetFontFile(parameter.fontFile);
    anan.SetTextColor(parameter.textColor);
    anan.SetFace((AnansFace)parameter.face);
    if(parameter.enableOutline)anan.EnableOutline();
    if(parameter.image != ""){
        if(parameter.text != L"")extent.x *= .5f;
        anan.AddImage(parameter.image, offset, extent);
    }
    if(parameter.text != L""){
        if(parameter.image != "")offset.x += extent.x;
        anan.AddText(parameter.text, offset, extent);
    }
    if(addHand)anan.AddHand();

    if(parameter.angle)anan.Rotate(parameter.angle);

    int32_t result = anan.SaveImage(parameter.out);
    if(result){
        printf("success to save image.\n");
    }
    else{
        printf("failed to save image. error code is %d\n", result);
    }
}
