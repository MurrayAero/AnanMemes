#ifndef FONTS_HPP
#define FONTS_HPP
#include <cmath>
#include <vector>
#include <stdint.h>
#include <iostream>
#include "glm/glm.hpp"
#include "stb_truetype.h"
#ifndef ROW_COLUMN_TO_INDEX
#define ROW_COLUMN_TO_INDEX(ROW_INDEX, COLUMN_INDEX, COLUMN)((ROW_INDEX) * (COLUMN) + (COLUMN_INDEX))
#endif
namespace fonts{
    struct FontAttribute{
        glm::uvec2 size;
        glm::uvec2 offset;
    };
    static inline auto GetFontWidth(const std::wstring& text, uint32_t start, const std::vector<FontAttribute>& fontInfo) {
        if (start >= fontInfo.size() || text.empty()) {
            return 0;
        }
        
        uint32_t end = std::min(static_cast<uint32_t>(start + text.length()), static_cast<uint32_t>(fontInfo.size()));
        if (start >= end) return 0;
        
        int32_t startX = fontInfo[start].offset.x;
        int32_t endX = fontInfo[end - 1].offset.x + fontInfo[end - 1].size.x;
        
        int32_t totalWidth = endX - startX;
        
        return totalWidth;
    }
    static inline std::vector<FontAttribute> GenerateFontBitmap(const wchar_t* text, uint32_t len, stbtt_fontinfo* info, float pixel, unsigned char* outBitmap, int bitmapWidth) {
        float scale = stbtt_ScaleForPixelHeight(info, pixel);

        int32_t ascent, descent, lineGap;
        stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);
        ascent = roundf(ascent * scale);
        
        std::vector<FontAttribute> fontInfo(len);
        
        int cursor = 0;
        for (uint32_t i = 0; i < len; ++i) {
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(info, text[i], &advanceWidth, &leftSideBearing);
            
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(info, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
            
            int y_offset = ascent + c_y1;
            int x_offset = cursor + roundf(leftSideBearing * scale);
            int byteOffset = x_offset + y_offset * bitmapWidth;
            
            stbtt_MakeCodepointBitmap(info, outBitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bitmapWidth, scale, scale, text[i]);
            
            cursor += roundf(advanceWidth * scale);
            
            if (i + 1 < len) {
                int kern = stbtt_GetCodepointKernAdvance(info, text[i], text[i + 1]);
                cursor += roundf(kern * scale);
            }
            fontInfo[i].offset = glm::uvec2(x_offset, y_offset);
            fontInfo[i].size = glm::uvec2(c_x2 - c_x1, c_y2 - c_y1);
        }
        return fontInfo;
    }
    static inline std::vector<FontAttribute> GenerateFont(const unsigned char *fontData, int width, const wchar_t word[], uint32_t len, unsigned char *out){
        stbtt_fontinfo info;
        int offset = stbtt_GetFontOffsetForIndex(fontData, 0);
        if (offset < 0) {
            printf("无效的TTC索引\n");
            return {};
        }
        int result = stbtt_InitFont(&info, fontData, offset);
        if(!result){
            printf("in function:%s, stbtt_InitFont error, result = %d\n", __FUNCTION__, result);
            return {};
        }
        float pixels = width / len;
        return GenerateFontBitmap(word, len, &info, pixels, out, width);
    }
    //每个字体均对齐
    static inline std::vector<FontAttribute> GetFontImageData(const unsigned char *fontData, int width, int height, const wchar_t word[], uint32_t len, unsigned char *out) {
        stbtt_fontinfo info;
        int offset = stbtt_GetFontOffsetForIndex(fontData, 0);
        if (offset < 0) {
            printf("无效的TTC索引\n");
            return {};
        }
        int result = stbtt_InitFont(&info, fontData, offset);
        if(!result){
            printf("in function:%s, stbtt_InitFont error, result = %d\n", __FUNCTION__, result);
            return {};
        }
        
        int cellWidth = width / len;
        int cellHeight = height;
        
        float scale = stbtt_ScaleForPixelHeight(&info, cellHeight * 0.8f);
        
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        
        memset(out, 0, width * height);
        
        std::vector<FontAttribute> fontInfo(len); // x: 水平偏移, y: 宽度, z: 高度, w: 垂直偏移
        for (uint32_t i = 0; i < len; ++i) {
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(&info, word[i], &advanceWidth, &leftSideBearing);
            
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
            
            int charWidth = c_x2 - c_x1;
            int charHeight = c_y2 - c_y1;
            
            // 计算字符在格子中的居中位置
            int x_offset = i * cellWidth + (cellWidth - charWidth) / 2;
            int y_offset = (cellHeight - charHeight) / 2;
            
            if (x_offset + charWidth <= width && y_offset + charHeight <= height) {
                int byteOffset = x_offset + (y_offset * width);
                stbtt_MakeCodepointBitmap(&info, out + byteOffset, charWidth, charHeight, width, scale, scale, word[i]);
            }
            fontInfo[i].offset = glm::uvec2(x_offset, y_offset);
            fontInfo[i].size = glm::uvec2(c_x2 - c_x1, c_y2 - c_y1);
        }
        return fontInfo;
    }
    static inline void SetColor(const unsigned char *data, const glm::uvec2&size, const glm::uvec3&color, unsigned char *out, const glm::uvec2&outSize, uint32_t width_offset){
        const uint32_t channels = 4;
        const uint32_t imageSize = size.x * size.y * channels;
        for (uint64_t y = 0; y < outSize.y; ++y){
            for (uint64_t x = 0; x < outSize.x; ++x){
                const uint64_t idx = ROW_COLUMN_TO_INDEX(y, x + width_offset, outSize.x) * channels;
#ifdef DEBUG
                if (idx + 3 >= imageSize){
                    printf("idx + 3(%d) >= imageSize(%d)\n", idx + 3, imageSize);
                }
#endif
                if (data[idx]){
                    out[idx] = color.r;
                    out[idx + 1] = color.g;
                    out[idx + 2] = color.b;
                    out[idx + 3] = 0xff;
                }
            }
        }
    }
}
#endif