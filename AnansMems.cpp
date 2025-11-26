#include <array>
#include <assert.h>
#include <memory.h>

#include "stb_image.h"
#include "ananImage.hpp"
#include "ananString.hpp"
#include "AnansMems.hpp"
#include "stb_image_write.h"
#include "stb_image_resize2.h"
uint32_t GetFileSize(FILE *fp){
    uint32_t size = 0;
    if(fp){
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }
    return size;
}
uint32_t GetLine(const std::wstring& text, uint32_t fontSize, const glm::uvec2& extent) {
    if (text.empty() || fontSize == 0 || extent.x == 0) {
        return 0;
    }
    uint32_t max_chars_per_line = extent.x / fontSize;
    if (max_chars_per_line == 0) {
        return 0;
    }
    uint32_t line_count = (text.length() + max_chars_per_line - 1) / max_chars_per_line;
    
    return line_count > 0 ? line_count : 1;
}
// uint32_t AnansMemes::GetInvalidPixelSize(const std::wstring &text){
//     const uint32_t symbolCount = countSymbols(text);
//     const uint32_t chineseCount = text.size() - symbolCount;
//     const uint32_t fontWidth = MAX_FONT_WIDTH * text.length();
//     const uint32_t chineseWidth = MAX_FONT_WIDTH - 5, symbolWidth = MAX_FONT_WIDTH / 2 - 2;
//     const uint32_t realFontWidth = chineseCount * chineseWidth + symbolCount * symbolWidth;
//     return fontWidth - realFontWidth;
// }
bool isOverlapping(const glm::uvec2& pos, const glm::uvec2& size, const std::vector<fonts::FontAttribute>& fontInfo){
    uint32_t newLeft = pos.x;
    uint32_t newRight = pos.x + size.x;
    uint32_t newTop = pos.y;
    uint32_t newBottom = pos.y + size.y;
    
    for (const auto& fontAttr : fontInfo) {
        uint32_t existingLeft = fontAttr.offset.x;
        uint32_t existingRight = fontAttr.offset.x + fontAttr.size.x;
        uint32_t existingTop = fontAttr.offset.y;
        uint32_t existingBottom = fontAttr.offset.y + fontAttr.size.y;
        
        // 检查矩形是否重叠
        // 两个矩形不重叠的条件是：一个在另一个的左边、右边、上边或下边
        bool notOverlapping = (newRight <= existingLeft) ||  // 新矩形在已存在矩形的左边
                             (newLeft >= existingRight) ||   // 新矩形在已存在矩形的右边
                             (newBottom <= existingTop) ||   // 新矩形在已存在矩形的上边
                             (newTop >= existingBottom);     // 新矩形在已存在矩形的下边
        
        if (!notOverlapping) {
            return true;
        }
    }
    return false;
}
bool inExtent(const glm::uvec2& pos, const glm::uvec2& size, const glm::uvec2& offset, const glm::uvec2& extent) {
    return pos.x >= offset.x && pos.y >= offset.y && pos.x + size.x <= offset.x + extent.x && pos.y + size.y <= offset.y + extent.y;
}
std::string AnansMemes::GetAnansImageName(AnansFace face){
    switch (face){
    case AnansFace::Yandere:
        return "Yandere.png";
    case AnansFace::Blush:
        return "Blush.png";
    case AnansFace::Happy:
        return "Happy.png";
    case AnansFace::Angry:
        return "Angry.png";
    case AnansFace::Speechless:
        return "Speechless.png";
    case AnansFace::MiniHappy:
        return "miniHappy.jpg";
    case AnansFace::MiniTsundere:
        return "miniTsundere.jpg";
    case AnansFace::MiniBase:
        return "miniBase.jpg";
    case AnansFace::Tsundere:
        return "Tsundere.png";
    default:
        break;
    }
    return "Base.png";
}
bool AnansMemes::GetFontData(const std::string&fontPath, const std::wstring &text){
	long int size = 0;
	unsigned char *fontBuffer = NULL;   
	FILE *fontFile = fopen(fontPath.c_str(), "rb");
	if(fontFile){
		size = GetFileSize(fontFile);
		fontBuffer = (unsigned char *)malloc(size);
		fread(fontBuffer, size, 1, fontFile);
	}
	else{
		perror("open font file error");
		printf("file is %s\n", fontPath.c_str());
        return false;
	}
    const uint32_t fontCount = text.length();
    const uint32_t fontSize = MAX_FONT_WIDTH * fontCount * MAX_FONT_HEIGHT;
    stbi_uc *fontData = new stbi_uc[fontSize];
    memset(fontData, 0, fontSize);
    fontInfo = fonts::GenerateFont(fontBuffer, MAX_FONT_WIDTH * fontCount, text.data(), fontCount, fontData);
    // stbi_write_bmp("fonttest.bmp", MAX_FONT_WIDTH * fontCount, MAX_FONT_HEIGHT, 1, fontData);
    fclose(fontFile);
    mFontData = new stbi_uc[fontSize * 4];
    ananImage::convertFontToRGBA(fontData, glm::uvec2(MAX_FONT_WIDTH * text.length(), MAX_FONT_HEIGHT), mFontData);
    delete[] fontData;
    if (fontBuffer) {
        free(fontBuffer);
        fontBuffer = nullptr;
    }
    return true;
}
bool AnansMemes::GetAnanImageData(const std::string &path, AnansFace face){
    if(anan.data){
        delete[]anan.data;
        anan.data = nullptr;
    }
    if(hand.data){
        delete[]hand.data;
        hand.data = nullptr;
    }
    std::string ananimage = GetAnansImageName(face);
    char imageName[MAX_PATH];
    int32_t c;
    sprintf(imageName, "%s%s", path.c_str(), ananimage.c_str());
    stbi_uc *data = stbi_load(imageName, &anan.size.x, &anan.size.y, &c, STBI_rgb_alpha);
    if(!data){
        printf("load image error, image name %s\n", imageName);
        return false;
    }
    anan.channels = 4;
    const uint32_t ananImageSize = anan.size.x * anan.size.y * anan.channels;
    anan.data = new stbi_uc[ananImageSize];
    memcpy(anan.data, data, ananImageSize);
    stbi_image_free(data);
    return true;
}
AnansMemes::AnansMemes(/* args */){
}

AnansMemes::AnansMemes(const std::string &path){
    if(path[0] == '~'){
        currentPath = getenv("HOME") + path.substr(1);
    }
    else{
        currentPath = path;
    }
}

AnansMemes::AnansMemes(const std::string&path, AnansFace face){
    if(path[0] == '~'){
        currentPath = getenv("HOME") + path;
    }
    else{
        currentPath = path;
    }
    if(!GetAnanImageData(path, face)){
        throw "load image error";
    }
}

AnansMemes::~AnansMemes(){
    if(anan.data){
        delete[]anan.data;
        anan.data = nullptr;
    }
    if(mFontData){
        delete[]mFontData;
        mFontData = nullptr;
    }
    if(hand.data){
        delete[]hand.data;
        hand.data = nullptr;
    }
    if(doodle.data){
        delete[]doodle.data;
        doodle.data = nullptr;
    }
}
bool AnansMemes::AddHand(){
    int32_t c;
    char imageName[MAX_PATH];
    if(face == AnansFace::Tsundere){
        sprintf(imageName, "%s%sHand_Tsundere.png", currentPath.c_str(), IMAGE_PATH);
    }
    else{
        //目前的程序来看, 只能是一般表情
        sprintf(imageName, "%s%sHand.png", currentPath.c_str(), IMAGE_PATH);
    }
    stbi_uc *data = stbi_load(imageName, &hand.size.x, &hand.size.y, &c, STBI_rgb_alpha);
    if(!data){
        printf("load image error, image name %s\n", imageName);
        return false;
    }
    hand.channels = 4;
    const uint32_t handImageSize = hand.size.x * hand.size.y * hand.channels;
    hand.data = new stbi_uc[handImageSize];
    memcpy(hand.data, data, handImageSize);
    stbi_image_free(data);

    //之前是因为改图片的时候忘记保留alpha通道, 现在不需要了
    // ananImage::removeInvalidPixel(hand.data, hand.size);
    
    ananImage::copy(hand.data, anan.data, hand.size, hand.size, hand.size, glm::uvec2(0), glm::uvec2(0));
    return true;
}
bool AnansMemes::AddText(const std::wstring &text, const glm::uvec2 &offset, const glm::uvec2 &extent){
    if(!GetFontData(currentPath + FONT_PATH + "ukai.ttc", text)){
        return false;
    }

    SetTextColor(text);

    uint32_t index = 0, currentFontIndex = 0;
    const uint32_t fontWidth = MAX_FONT_WIDTH * static_cast<uint32_t>(text.length()), line = GetLine(text, MAX_FONT_WIDTH, extent);
    glm::uvec2 fontImageOffset = {}, fontOffset, fontSzie = glm::uvec2(fontWidth, MAX_FONT_HEIGHT), cutSize = glm::uvec2(MAX_FONT_HEIGHT);

    auto split = ananStr::split(text, line);

    fontImageOffset.x = fontInfo[0].offset.x;
    for (auto&it:split){
        uint32_t currentFontWidth = fonts::GetFontWidth(it, currentFontIndex, fontInfo);
        if(currentFontWidth >= extent.x){
            char message[MAX_BYTE];
            sprintf(message, "currentFontWidth(%d) >= extent.x(%d)", currentFontWidth, extent.x);
            throw std::out_of_range(message);
        }
        glm::uvec2 currentFontSize = glm::uvec2(currentFontWidth, MAX_FONT_HEIGHT);
        const glm::uvec2 max = glm::uvec2(extent.x > currentFontSize.x ? extent.x - currentFontSize.x : 0, extent.y > currentFontSize.y ? extent.y - currentFontSize.y : 0);
        int32_t randCount = 0;
        //TODO:应该改进随机算法和下面的判断函数
        do{
            fontOffset.x = offset.x + (max.x > 0 ? rand() % max.x : 0);
            fontOffset.y = offset.y + (max.y > 0 ? rand() % max.y : 0);
            ++randCount;
        }while((!inExtent(fontOffset, currentFontSize, offset, extent)  || isOverlapping(fontOffset, currentFontSize, mMediaLayout)) && randCount < MAX_RANDOM_COUNT);
        if(randCount >= MAX_RANDOM_COUNT){
            wprintf(L"randCount >= MAX_RANDOM_COUNT(%d), ignore current and all subsequent strings\n", MAX_RANDOM_COUNT);
            break;
        }
        cutSize.x = currentFontWidth;

        if(face >= AnansFace::Tsundere){
            glm::ivec2 new_size = ananImage::calculateRotatedSize(face == AnansFace::Tsundere?TSUNDERE_ANAN_ANGLE:MINI_ANAN_ANGLE, cutSize);
            const uint32_t font_data_size = cutSize.x * cutSize.y * 4, new_data_size = new_size.x * new_size.y * 4;
            stbi_uc * font_data = new stbi_uc[font_data_size], *new_data = new stbi_uc[new_data_size];
            memset(font_data, 0, font_data_size);
            memset(new_data, 0, new_data_size);
            ananImage::copy(mFontData, font_data, cutSize, fontSzie, cutSize, fontImageOffset, glm::uvec2(0));
            ananImage::rotate(font_data, cutSize, face == AnansFace::Tsundere?TSUNDERE_ANAN_ANGLE:MINI_ANAN_ANGLE, new_data, new_size);
            // stbi_write_png("rotate_font.png", cutSize.x, cutSize.y, 4, font_data, 0);
            // stbi_write_png("rotate_new_font.png", new_size.x, new_size.y, 4, new_data, 0);
#ifdef DEBUG
            printf("fontOffset = %d, %d\ncutSize = %d, %d\nnew_size = %d, %d\n", fontOffset.x, fontOffset.y, cutSize.x, cutSize.y, new_size.x, new_size.y);
#endif
            ananImage::copy(new_data, anan.data, new_size, new_size, anan.size, glm::uvec2(0), fontOffset);
            delete[]font_data;
            delete[]new_data;
        }
        else{
            ananImage::copy(mFontData, anan.data, cutSize, fontSzie, anan.size, fontImageOffset, fontOffset);
        }

        fonts::FontAttribute pos;
        pos.offset = fontOffset;
        //这个宽度和高度主要用于判断是否重叠，因此需要更准确的数据
        pos.size = glm::uvec2(currentFontWidth, fontInfo[index].size.y);
        mMediaLayout.push_back(pos);

        currentFontIndex += it.length();

        if(index != split.size() - 1 && ananStr::GetFirstPunctuationPos(text) != std::string::npos && split[index + 1][0] != '['){
            ++currentFontIndex;
        }
        ++index;
        if(currentFontIndex < fontInfo.size())fontImageOffset.x = fontInfo[currentFontIndex].offset.x;
    }
    return true;
}
// bool AnansChatBox::AddText(const std::wstring&text, const glm::uvec2&offset, const glm::uvec2&extent){
//     const uint32_t line = CalcFontSize(text, extent);
//     if(!GetFontData(currentPath + FONT_PATH + "ukai.ttc", text)){
//         return false;
//     }
//     SetTextColor(text);
//     // glm::uvec2 size = fonts.size;
//     // size.x = MAX_FONT_WIDTH * text.length();
//     // SetTextColor(mFontData, size, COMMAND_COLOR, fonts.data, glm::uvec2(MAX_FONT_WIDTH - 5, MAX_FONT_HEIGHT), 0);
//     const uint32_t imageSize = anan.size.x * anan.size.y * anan.channels;
//     //尝试用智能指针
//     //auto up = std::make_unique<Foo>(42);  
//     //auto arr = std::make_unique<int[]>(10);返回的指针支持数组索引
//     //如果有可能，将该类分成多个功能类
//     /*
//         #include <memory>
//         #include <cstdlib>
//         int main() {
//             // unique_ptr + free
//             std::unique_ptr<char, decltype(&std::free)> up(
//                 static_cast<char*>(std::malloc(256)), &std::free);
//             // shared_ptr + free
//             std::shared_ptr<char> sp(
//                 static_cast<char*>(std::malloc(512)), &std::free);
//             return 0;
//         }
//     */
//     //目前只换行不改字体大小
//     glm::uvec2 fontOffset = {}, chatOffset;
//     uint32_t index = 0;
//     // const uint32_t invalidPixel = GetInvalidPixelSize(text);
//     const uint32_t fontWidth = MAX_FONT_WIDTH * static_cast<uint32_t>(text.length());
//     const uint32_t chineseWidth = MAX_FONT_WIDTH - 5, symbolWidth = MAX_FONT_WIDTH / 2;
//     auto split = ananStr::split(text, line);
//     for (auto&it:split){
//         const uint32_t symbolCount = countSymbols(it);
//         const uint32_t chineseCount = it.length() - symbolCount;
//         const uint32_t currentFontWidth = chineseCount * chineseWidth + symbolCount * symbolWidth;
//         if(currentFontWidth >= extent.x){
//             char message[MAX_BYTE];
//             sprintf(message, "currentFontWidth(%d) >= extent.x(%d)", currentFontWidth, extent.x);
//             throw std::out_of_range(message);
//         }
//         //判断文字以及它们的宽度是否在范围内
//         do{
//             chatOffset.x = offset.x + rand() % (offset.x + extent.x - offset.x);
//             chatOffset.y = offset.y + rand() % (offset.y + extent.y - offset.y);
//         }while(!inExtent(chatOffset, glm::uvec2(currentFontWidth, MAX_FONT_HEIGHT), offset, extent)  || isOverlapping(chatOffset, mMediaLayout));
//         glm::uvec4 pos;
//         pos.x = chatOffset.x;
//         pos.y = chatOffset.y;
//         pos.z = currentFontWidth;
//         pos.w = MAX_FONT_HEIGHT;
//         mMediaLayout.push_back(pos);
//         // chatOffset.x = offset.x + (extent.x - currentFontWidth) / 2;// + invalidPixel;
//         // chatOffset.y = offset.y + (extent.y / 2 - line * (MAX_FONT_HEIGHT / 2)) * (index + 1);
//         copy(mFontData, anan.data, glm::uvec2(currentFontWidth, MAX_FONT_HEIGHT), glm::uvec2(fontWidth, MAX_FONT_HEIGHT), anan.size, fontOffset, chatOffset);
//         ++index;
//         fontOffset.x += currentFontWidth;
//     }
//     return true;
// }

bool AnansMemes::AddImage(const std::string &image, const glm::uvec2&offset, const glm::uvec2&extent){
    //给新安安添加的图片也需要旋转
    if(doodle.data){
        delete[]doodle.data;
        doodle.data = nullptr;
    }
    int32_t width, height, c;
    stbi_uc *data = stbi_load(image.c_str(), &width, &height, &c, STBI_rgb_alpha);
    if(!data){
        printf("load image error, image name %s\n", image.c_str());
        return false;
    }
    glm::uvec2 imageSize, imageOffset;
    ananImage::calculateImageSize(extent, glm::uvec2(width, height), imageSize);
    doodle.channels = 4;
    doodle.size = imageSize;
    const uint32_t uImageSize = imageSize.x * imageSize.y * doodle.channels;
    doodle.data = new stbi_uc[uImageSize];

    if(imageSize != glm::uvec2(width, height)){
        stbir_resize(data, width, height, 0, doodle.data, imageSize.x, imageSize.y, 0, STBIR_RGBA, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM);
    }

    if(face == AnansFace::Tsundere){
        imageSize = ananImage::calculateRotatedSize(TSUNDERE_ANAN_ANGLE, imageSize);
        stbi_uc *temp = new stbi_uc[imageSize.x * imageSize.y * doodle.channels];
        ananImage::rotate(doodle.data, doodle.size, TSUNDERE_ANAN_ANGLE, temp, imageSize);
        delete[]doodle.data;
        doodle.data = temp;
        doodle.size = imageSize;
    }

    const glm::uvec2 max = glm::uvec2(extent.x > MAX_FONT_WIDTH ? extent.x - MAX_FONT_WIDTH : 0, extent.y > MAX_FONT_HEIGHT ? extent.y - MAX_FONT_HEIGHT : 0);
    do{       
        imageOffset.x = offset.x + (max.x > 0 ? rand() % max.x : 0);
        imageOffset.y = offset.y + (max.y > 0 ? rand() % max.y : 0);
    }while(!inExtent(imageOffset, imageSize, offset, extent)  || isOverlapping(imageOffset, imageSize, mMediaLayout));

    mMediaLayout.push_back({imageSize, imageOffset});

    ananImage::copy(doodle.data, anan.data, doodle.size, doodle.size, anan.size, glm::uvec2(0), imageOffset);

    stbi_image_free(data);
    return false;
}

int32_t AnansMemes::SaveImage(const std::string &image){
    if(strstr(image.c_str(), ".jpg")){
        return stbi_write_jpg(image.c_str(), anan.size.x, anan.size.y, 4, anan.data, 0);
    }
    return stbi_write_png(image.c_str(), anan.size.x, anan.size.y, 4, anan.data, 0);
}

bool AnansMemes::SetFace(AnansFace face){
    this->face = face;
    return GetAnanImageData(currentPath + IMAGE_PATH, face);
}
void AnansMemes::SetTextColor(const std::wstring&text){
    glm::uvec3 color;
    glm::uvec2 size, fontSize;
    std::vector<std::wstring>split;
    uint32_t currentFontInfo = 0, offset = fontInfo[0].offset.x;

    ananStr::split(text, split);
    fontSize.x = MAX_FONT_WIDTH * text.length();
    fontSize.y = MAX_FONT_HEIGHT;
    for (auto&it:split){
        if(it[0] == L'['){
            color = COMMAND_COLOR;
        }
        else{
            color = glm::uvec3(0);
        }
        size.x = fonts::GetFontWidth(it, currentFontInfo, fontInfo);
        fonts::SetColor(mFontData, fontSize, color, mFontData, size, offset);
        currentFontInfo += it.length();
        if(currentFontInfo < fontInfo.size())offset = fontInfo[currentFontInfo].offset.x;
    }
    // stbi_write_png("fonttest.png", MAX_FONT_WIDTH * text.length(), MAX_FONT_HEIGHT, 4, mFontData, 0);
}
// void AnansChatBox::SetTextColor(const std::wstring&text){
//     uint32_t currentFontCount = 0;
//     std::vector<std::wstring>input;
//     ananStr::split(text, input);

//     uint32_t offset = 0;
//     glm::uvec2 size;
//     size.y = MAX_FONT_HEIGHT;
//     const uint32_t chineseWidth = MAX_FONT_WIDTH - 5, symbolWidth = MAX_FONT_WIDTH / 2 - 2;
//         uint32_t fontBufferWidth = MAX_FONT_WIDTH * static_cast<uint32_t>(text.length());
//     for (auto&it:input){
//         uint32_t symbolCount = countSymbols(it);
//         uint32_t chineseCount = it.size() - symbolCount;
//         glm::uvec3 color;
//         if(it[0] == L'['){
//             color = COMMAND_COLOR;
//         }
//         else{
//             color = glm::uvec3(0);
//         }
//         size.x = chineseWidth * chineseCount + symbolWidth * symbolCount;
//         fonts::SetColor(mFontData, glm::uvec2(fontBufferWidth, MAX_FONT_HEIGHT), color, mFontData, size, offset);
//         offset += size.x;
//     }
// }