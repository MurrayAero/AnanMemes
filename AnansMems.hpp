#ifndef ANANS_ANAN_MEMS_HPP
#define ANANS_ANAN_MEMS_HPP
#include <string>
#include <vector>
#include "fonts.hpp"
#include "stb_image.h"
#include "glm/glm.hpp"
#ifndef MAX_BYTE
#define MAX_BYTE 0xff
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifdef _WIN32
#define FONT_PATH "resource\\fonts\\"
#define IMAGE_PATH "resource\\image\\" 
#else
#define FONT_PATH "resource/fonts/"
#define IMAGE_PATH "resource/image/" 
#endif
#define MINI_ANAN_TEXT_ANGLE -20
#define MINI_ANAN_EXQUISITE_TEXT_ANGLE -15
#define COMMAND_COLOR glm::uvec3(130, 0, 130)
#define IS_ODD(NUM) ((NUM) % 2)
#define IS_EVEN(NUM) (!IS_ODD(NUM))
#define MAX_FONT_WIDTH 50u
#define MAX_FONT_HEIGHT MAX_FONT_WIDTH
enum class AnansFace{
    Base = 0,
    Happy,
    Yandere,
    Blush,
    Angry,
    Speechless,
    MiniHappy,
    MiniTsundere,
    MiniBase,
    miniTsundere_Exquisite
};
struct AnanImage{
    stbi_uc *data;
    glm::ivec2 size;
    int32_t channels;
};
class AnansMemes{
    AnansFace face;
    std::string currentPath;
    stbi_uc *mFontData = nullptr;
    AnanImage anan = {}, doodle = {}, hand = {};

    std::vector<fonts::FontAttribute>mMediaLayout;
    
    std::vector<fonts::FontAttribute>fontInfo;
    // uint32_t GetInvalidPixelSize(const std::wstring&text);

    std::string GetAnansImageName(AnansFace face);

    bool GetFontData(const std::string&fontPath, const std::wstring&text);
    
    bool GetAnanImageData(const std::string&path, AnansFace face);

    void SetTextColor(const std::wstring&text);
public:
    AnansMemes();
    AnansMemes(const std::string&path);
    AnansMemes(const std::string&path, AnansFace face);
    ~AnansMemes();

    bool AddHand(const std::string&fileName);
    //offset是白板的左上角, extent是白板的大小
    bool AddText(const std::wstring&text, const glm::uvec2&offset, const glm::uvec2&extent);
    bool AddImage(const std::string&image, const glm::uvec2&offset, const glm::uvec2&extent);
    
    int32_t SaveImage(const std::string &image);

    bool SetFace(AnansFace face);
};
#endif