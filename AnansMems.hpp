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
#define MAX_RANDOM_COUNT 500

#define NERVOUS_ANAN_ANGLE -11
#define TSUNDERE_ANAN_ANGLE -15

#define TEXT_MARGIN 5

#define OUTLINE_SIZE 4

#define COMMAND_COLOR glm::uvec3(130, 0, 130)

// #define IS_ODD(NUM) ((NUM) % 2)
// #define IS_EVEN(NUM) (!IS_ODD(NUM))

// #define MIN_FONT_WIDTH 50u
// #define MIN_FONT_HEIGHT MIN_FONT_WIDTH
enum class AnansFace{
    Happy = 0,
    Base,
    Yandere,
    Blush,
    Angry,
    Speechless,
    Tsundere,
    Nervous
};
struct AnanImage{
    stbi_uc *data;
    glm::ivec2 size;
    int32_t channels;
};
class AnansMemes{
    AnansFace face;
    float mFontSize;
    std::string fontFile;
    std::string currentPath;
    // glm::uvec2 mImageSize;
    stbi_uc *mFontData = nullptr;
    AnanImage anan = {}, doodle = {}, hand = {};
    glm::uvec3 defaultTextColor = glm::uvec3(0xff * .8);

    std::vector<fonts::FontAttribute>mLayout;
    
    std::vector<fonts::FontAttribute>fontInfo;
    // uint32_t GetInvalidPixelSize(const std::wstring&text);

    void CopyText(const glm::uvec2&fontOffset, const glm::uvec2 &currentFontSize, const glm::uvec2&fontSzie, const glm::uvec2&fontImageOffset);

    bool GetFontData(const std::string&fontPath, const std::wstring&text);
    
    bool GetAnanImageData(const std::string&path, AnansFace face);

    glm::uvec2 RandomPosition(const glm::uvec2 &offset, const glm::uvec2 &area, const glm::uvec2&currentFontSize);

    void SetTextColor(const std::wstring&text);
public:
    AnansMemes();
    AnansMemes(const std::string&path);
    AnansMemes(const std::string&path, AnansFace face);
    ~AnansMemes();

    bool AddHand();
    //offset是白板的左上角, extent是白板的大小
    bool AddText(const std::wstring&text, const glm::uvec2&offset, const glm::uvec2&area);
    bool AddImage(const std::string&image, const glm::uvec2&offset, const glm::uvec2&area);

    std::string GetAnansImageName(AnansFace face);
    
    void Rotate(float angle);
    
    int32_t SaveImage(const std::string &image);

    bool SetFace(AnansFace face);

    inline void SetFontFile(const std::string&fontFile)noexcept{
        this->fontFile = fontFile;
    }
    inline void SetTextColor(const glm::uvec3&color)noexcept{
        defaultTextColor = color;
    }
};
#endif