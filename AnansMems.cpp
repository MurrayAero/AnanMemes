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
//     const uint32_t fontWidth = mFontSize.x * text.length();
//     const uint32_t chineseWidth = mFontSize.x - 5, symbolWidth = mFontSize.x / 2 - 2;
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
bool inArea(const glm::uvec2& pos, const glm::uvec2& size, const glm::uvec2& offset, const glm::uvec2& extent) {
    return pos.x >= offset.x && pos.y >= offset.y && pos.x + size.x <= offset.x + extent.x && pos.y + size.y <= offset.y + extent.y;
}
std::vector<std::wstring> splitLines(const std::wstring& text) {
    std::vector<std::wstring> lines;
    std::wstring currentLine;
    
    for (wchar_t ch : text) {
        if (ananStr::IsLineBreak(ch)) {
            lines.push_back(currentLine);
            currentLine.clear();
        } else {
            currentLine += ch;
        }
    }
    
    // 添加最后一行
    if (!currentLine.empty() || text.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}
float calculateFontSize(const glm::uvec2& area, const std::wstring& text) {
    std::vector<std::wstring> lines = splitLines(text);
    const uint32_t lineCount = lines.size();
    // 根据高度计算字体大小
    float fontSize = (area.y - TEXT_MARGIN * 2) / lineCount;
    
    size_t maxChars = 0;
    for (const auto& line : lines) {
        maxChars = std::max(maxChars, line.length());
    }
    
    if (maxChars > 0) {
        float neededWidth = maxChars * fontSize;
        if (neededWidth > static_cast<float>(area.x)) {
            fontSize = fontSize * static_cast<float>(area.x) / neededWidth;
        }
    }
    
    return fontSize;
}
std::vector<glm::uvec2> layoutTexts(const glm::uvec2& offset, const glm::uvec2& area, const std::vector<glm::uvec2>& textSize) {
    std::vector<glm::uvec2> positions;
    positions.reserve(textSize.size());
    
    if (textSize.empty()) {
        return positions;
    }
    
    const uint32_t spacing = 1;
    
    // 只使用垂直排列，每个文本占一行
    uint32_t totalHeight = 0;
    for (const auto& size : textSize) {
        totalHeight += size.y;
    }
    // 添加行间距
    if (textSize.size() > 1) {
        totalHeight += (textSize.size() - 1) * spacing;
    }
    
    // 计算起始位置（垂直居中）
    glm::uvec2 current = glm::uvec2(
        offset.x + area.x / 2,  // 水平居中
        offset.y + (area.y - totalHeight) / 2  // 垂直居中
    );
    
    for (const auto& size : textSize) {
        uint32_t posX = current.x - size.x / 2;  // 水平居中
        positions.emplace_back(posX, current.y);
        current.y += size.y + spacing;  // 移动到下一行
    }
    
    return positions;
}
void AnansMemes::CopyText(const glm::uvec2&fontOffset, const glm::uvec2 &currentFontSize, const glm::uvec2&fontSzie, const glm::uvec2&fontImageOffset){
    const glm::uvec2 cutSize = glm::uvec2(currentFontSize.x, mFontSize);
    if(face >= AnansFace::Tsundere){
        const float angle = face == AnansFace::Tsundere?TSUNDERE_ANAN_ANGLE:NERVOUS_ANAN_ANGLE;
        glm::ivec2 new_size = ananImage::calculateRotatedSize(angle, cutSize);
        const uint32_t font_data_size = cutSize.x * cutSize.y * 4, new_data_size = new_size.x * new_size.y * 4;
        stbi_uc *font_data = new stbi_uc[font_data_size], *new_data = new stbi_uc[new_data_size];
        memset(font_data, 0, font_data_size);
        memset(new_data, 0, new_data_size);
        ananImage::copy(mFontData, font_data, cutSize, fontSzie, cutSize, fontImageOffset);
        ananImage::rotate(font_data, cutSize, angle, new_data, new_size);
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
    case AnansFace::Tsundere:
        return "Tsundere.png";
    case AnansFace::Nervous:
        return "Nervous.jpg";
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
    const uint32_t fontSize = mFontSize * fontCount * mFontSize;
    stbi_uc *fontData = new stbi_uc[fontSize];
    memset(fontData, 0, fontSize);
    if(mEnableOutline){
        int minOutline = 5;
        int maxOutline = 10;
        float outlineFactor = 50.0f;
        mOutlineSize = std::max(minOutline, std::min(maxOutline, static_cast<int>(outlineFactor / mFontSize + 0.5f)));
    }
    fontInfo = fonts::GenerateFont(fontBuffer, glm::ivec2(mFontSize * fontCount, mFontSize), text.data(), fontCount, fontData, mOutlineSize);
    // stbi_write_bmp("fonttest.bmp", mFontSize * fontCount, mFontSize, 1, fontData);
    fclose(fontFile);
    mFontData = new stbi_uc[fontSize * 4];
    ananImage::convertFontToRGBA(fontData, glm::uvec2(mFontSize * text.length(), mFontSize), mFontData);
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
    int32_t c;
    char imageName[MAX_PATH];
    std::string ananimage = GetAnansImageName(face);
    if(mAnanImage != ""){
        strcpy(imageName, mAnanImage.c_str());
    }
    else{
        sprintf(imageName, "%s%s", path.c_str(), ananimage.c_str());
    }
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
glm::uvec2 AnansMemes::RandomPosition(const glm::uvec2 &offset, const glm::uvec2 &area, const glm::uvec2&currentFontSize){
    uint32_t randCount = 0;
    glm::uvec2 position = glm::uvec2(0);
    const glm::uvec2 max = glm::clamp(area - currentFontSize, glm::uvec2(0), area);//glm::uvec2(extent.x > currentFontSize.x ? extent.x - currentFontSize.x : 0, extent.y > currentFontSize.y ? extent.y - currentFontSize.y : 0);
    //TODO:应该改进随机算法和下面的判断函数
    do{
        ++randCount;
        position.x = offset.x + (max.x > 0 ? rand() % max.x : 0);
        position.y = offset.y + (max.y > 0 ? rand() % max.y : 0);
    }while((!inArea(position, currentFontSize, offset, area) || isOverlapping(position, currentFontSize, mLayout)) && randCount < MAX_RANDOM_COUNT);
    if(randCount >= MAX_RANDOM_COUNT){
        printf("failed to find non-overlapping position for text after %d attempts, using last position\n", randCount);
    }
    return position;
}
AnansMemes::AnansMemes(/* args */){
}

AnansMemes::AnansMemes(const std::string &path){
    currentPath = path;
}

AnansMemes::AnansMemes(const std::string&path, AnansFace face){
    currentPath = path;
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
    else if(face == AnansFace::Nervous){
        sprintf(imageName, "%s%sHand_Nervous.png", currentPath.c_str(), IMAGE_PATH);
    }
    else{
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
    
    ananImage::copy(hand.data, anan.data, hand.size, hand.size, anan.size);
    return true;
}
bool AnansMemes::AddText(const std::wstring &text, const glm::uvec2 &offset, const glm::uvec2 &area){
    float scaleFactor = 1.0f;
    if(!mEnableOutline){
        if(face == AnansFace::Tsundere){
            scaleFactor = 0.7f;
        }
        else{
            scaleFactor = 0.9f;
        }
    }
    mFontSize = calculateFontSize(area, text) * scaleFactor;
    if(!GetFontData(currentPath + fontFile, text)){
        return false;
    }
    
    SetTextColor(text);
    //放置文本
    std::vector<glm::uvec2>fontImageOffset;
    uint32_t index = 0, currentFontIndex = 0;
    const uint32_t fontWidth = mFontSize * static_cast<uint32_t>(text.length()), line = GetLine(text, mFontSize, area);
    glm::uvec2 fontSzie = glm::uvec2(fontWidth, mFontSize);

    auto split = ananStr::split(text, line);

    std::vector<glm::uvec2>textSize;
    for (auto&it:split){
        fontImageOffset.push_back(glm::uvec2(fontInfo[currentFontIndex].offset.x, 0));
        textSize.push_back(glm::uvec2(fonts::GetFontWidth(it, currentFontIndex, fontInfo), mFontSize));

        if(index < split.size() - 1){
            currentFontIndex = text.find(split[index + 1], currentFontIndex);
        }
        ++index;
    }
    auto font_offset = layoutTexts(offset, area, textSize);

    for (uint32_t i = 0; i < split.size(); ++i){
        if(textSize[i].x >= area.x){
            char message[MAX_BYTE];
            sprintf(message, "textSize[%d].x(%d) >= extent.x(%d)", i, textSize[i].x, area.x);
            throw std::out_of_range(message);
        }
        const glm::uvec2 currentFontSize = glm::uvec2(textSize[i].x, mFontSize);

        CopyText(font_offset[i], currentFontSize, fontSzie, fontImageOffset[i]);

        fonts::FontAttribute pos;
        pos.offset = font_offset[i];
        //这个宽度和高度主要用于判断是否重叠，因此需要更准确的数据
        pos.size = glm::uvec2(textSize[i].x, fontInfo[i].size.y);
        mLayout.push_back(pos);
    }
    return true;
}

bool AnansMemes::AddImage(const std::string &image, const glm::uvec2&offset, const glm::uvec2&area){
    if(doodle.data){
        delete[]doodle.data;
        doodle.data = nullptr;
    }
    int32_t width, height, c;
    stbi_uc *data = stbi_load(image.c_str(), &width, &height, &c, STBI_rgb_alpha);
    if(!data){
        printf("load image error, image:%s\n", image.c_str());
        return false;
    }
    glm::uvec2 imageSize = ananImage::calculateImageSize(area, glm::uvec2(width, height));
    doodle.channels = 4;
    doodle.size = imageSize;
    const uint32_t uImageSize = imageSize.x * imageSize.y * doodle.channels;
    doodle.data = new stbi_uc[uImageSize];

    stbir_resize(data, width, height, 0, doodle.data, imageSize.x, imageSize.y, 0, STBIR_RGBA, STBIR_TYPE_UINT8, STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM);

    if(face >= AnansFace::Tsundere){
        const float angle = (face == AnansFace::Tsundere?TSUNDERE_ANAN_ANGLE:NERVOUS_ANAN_ANGLE);
        imageSize = ananImage::calculateRotatedSize(angle, imageSize);
        stbi_uc *temp = new stbi_uc[imageSize.x * imageSize.y * doodle.channels];
        ananImage::rotate(doodle.data, doodle.size, angle, temp, imageSize);
        delete[]doodle.data;
        doodle.data = temp;
        doodle.size = imageSize;
    }

    glm::uvec2 imageOffset = glm::uvec2(offset.x + (area.x - imageSize.x) / 2, offset.y + (area.y - imageSize.y) / 2);

    mLayout.push_back({0, imageSize, imageOffset});

    ananImage::copy(doodle.data, anan.data, doodle.size, doodle.size, anan.size, glm::uvec2(0), imageOffset);

    stbi_image_free(data);
    return false;
}

void AnansMemes::Rotate(float angle){
    const glm::ivec2 imageSize = ananImage::calculateRotatedSize(angle, anan.size);
    const uint32_t uImageSize = imageSize.x * imageSize.y * anan.channels;
    stbi_uc *temp = new stbi_uc[uImageSize];
    ananImage::rotate(anan.data, anan.size, angle, temp, imageSize);
    if(imageSize == anan.size){
        memcpy(anan.data, temp, uImageSize);
        delete[]temp;
    }
    else{
        anan.size = imageSize;
        delete[]anan.data;
        anan.data = temp;
    }
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
    if(split.empty()){
        split.push_back(text);
    }
    size.y = mFontSize;
    fontSize.y = mFontSize;
    fontSize.x = mFontSize * text.length();
    for (auto&it:split){
        if(it[0] == L'[' || it[0] == L'【'){
            color = COMMAND_COLOR;
        }
        else{
            color = defaultTextColor;
        }
        size.x = fonts::GetFontWidth(it, currentFontInfo, fontInfo);
        fonts::SetColor(mFontData, fontSize, color, mFontData, size, offset);
        currentFontInfo += it.length();
        if(currentFontInfo < fontInfo.size())offset = fontInfo[currentFontInfo].offset.x;
    }
    // stbi_write_png("fonttest.png", mFontSize.x * text.length(), mFontSize.y, 4, mFontData, 0);
}
// void AnansChatBox::SetTextColor(const std::wstring&text){
//     uint32_t currentFontCount = 0;
//     std::vector<std::wstring>input;
//     ananStr::split(text, input);

//     uint32_t offset = 0;
//     glm::uvec2 size;
//     size.y = mFontSize.y;
//     const uint32_t chineseWidth = mFontSize.x - 5, symbolWidth = mFontSize.x / 2 - 2;
//         uint32_t fontBufferWidth = mFontSize.x * static_cast<uint32_t>(text.length());
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
//         fonts::SetColor(mFontData, glm::uvec2(fontBufferWidth, mFontSize.y), color, mFontData, size, offset);
//         offset += size.x;
//     }
// }