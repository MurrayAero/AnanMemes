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
        int advance;
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
        
        return endX - startX;
    }
    static inline std::vector<FontAttribute> GenerateFontBitmap(const wchar_t* text, uint32_t len, stbtt_fontinfo* info, float pixel, int outlinePixels, unsigned char* outBitmap, const glm::ivec2&bitmapSize) {
        float scale = stbtt_ScaleForPixelHeight(info, pixel);
        
        int32_t ascent, descent, lineGap;
        stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);
        ascent = roundf(ascent * scale);
        
        std::vector<FontAttribute> fontInfo(len);
        
        // 水平排列：从左边开始，有描边时预留左边距
        int cursorX = (outlinePixels > 0) ? outlinePixels : 0;
        const int outline = std::max(0, outlinePixels);        
        for (uint32_t i = 0; i < len; ++i) {
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(info, text[i], &advanceWidth, &leftSideBearing);
            
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(info, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
            
            int glyphW = c_x2 - c_x1;
            int glyphH = c_y2 - c_y1;
            
            // 计算基准位置（水平排列）
            // x: 当前光标 + 左侧 bearing
            // y: baseline(ascent) + 字形相对基准线的偏移(c_y1)
            int glyphX = cursorX + roundf(leftSideBearing * scale);
            int glyphY = ascent + c_y1;
            
            // 有描边时向外扩展
            int renderX = glyphX;
            int renderY = glyphY;
            if (outline > 0) {
                renderX -= outline;
                renderY -= outline;
            }
            
            renderX = std::max(0, renderX);
            renderY = std::max(0, renderY);

            // 确保不超出位图底部
            if (renderY + glyphH + outline * 2 > bitmapSize.y) {
                renderY = std::max(0, bitmapSize.y - glyphH - outline * 2);
            }
            
            if (outline == 0) {
                // stride = bitmapSize.x（整行宽度）
                unsigned char* dest = outBitmap + renderY * bitmapSize.x + renderX;
                stbtt_MakeCodepointBitmap(info, dest, glyphW, glyphH, bitmapSize.x, scale, scale, text[i]);
            } else {
                std::vector<unsigned char> glyph(glyphW * glyphH, 0);
                // 渲染到临时缓冲区，stride = glyphW(紧密排列)
                stbtt_MakeCodepointBitmap(info, glyph.data(), glyphW, glyphH, glyphW, scale, scale, text[i]);
                
                // 1. 膨胀画描边(灰度 128)
                for (int gy = 0; gy < glyphH; ++gy) {
                    for (int gx = 0; gx < glyphW; ++gx) {
                        if (!glyph[gy * glyphW + gx]) continue;
                        
                        for (int oy = -outline; oy <= outline; ++oy) {
                            for (int ox = -outline; ox <= outline; ++ox) {
                                // 圆形核
                                if (ox*ox + oy*oy > outline*outline) continue;
                                
                                int dx = renderX + gx + ox;
                                int dy = renderY + gy + oy;
                                if (dx < 0 || dx >= bitmapSize.x || dy < 0 || dy >= bitmapSize.y) continue;
                                
                                unsigned char* pixel = outBitmap + dy * bitmapSize.x + dx;
                                if (*pixel < 128) *pixel = 128;
                            }
                        }
                    }
                }
                
                // 2. 覆盖文字（灰度 255）
                for (int gy = 0; gy < glyphH; ++gy) {
                    for (int gx = 0; gx < glyphW; ++gx) {
                        if (!glyph[gy * glyphW + gx]) continue;
                        int dx = renderX + gx;
                        int dy = renderY + gy;
                        if (dx < 0 || dx >= bitmapSize.x || dy < 0 || dy >= bitmapSize.y) continue;
                        outBitmap[dy * bitmapSize.x + dx] = 255;
                    }
                }
            }
            
            // 前进光标（水平方向）
            cursorX += roundf(advanceWidth * scale);
            
            // 字距调整
            if (i + 1 < len) {
                int kern = stbtt_GetCodepointKernAdvance(info, text[i], text[i + 1]);
                cursorX += roundf(kern * scale);
            }
            
            // 记录属性
            int totalW = glyphW + (outline > 0 ? outline * 2 : 0);
            int totalH = glyphH + (outline > 0 ? outline * 2 : 0);
            
            fontInfo[i].offset = glm::uvec2(renderX, renderY);
            fontInfo[i].size = glm::uvec2(totalW, totalH);
            fontInfo[i].advance = roundf(advanceWidth * scale);
        }
        
        return fontInfo;
    }

    static inline std::vector<FontAttribute> GenerateFont(const unsigned char *fontData, const glm::ivec2&size, const wchar_t word[], uint32_t len, unsigned char *out, int outlinePixels = 0){
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
        float pixels = size.x / len;
        return GenerateFontBitmap(word, len, &info, pixels, outlinePixels, out, size);
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
        for (uint64_t y = 0; y < outSize.y; ++y){
            for (uint64_t x = 0; x < outSize.x; ++x){
                const uint64_t idx = ROW_COLUMN_TO_INDEX(y, x + width_offset, size.x) * channels;
                if (data[idx] >= 150) {
                    out[idx] = color.r;
                    out[idx + 1] = color.g;
                    out[idx + 2] = color.b;
                    out[idx + 3] = 0xff;                    
                }
                else if (data[idx] >= 100) {
                    out[idx] = 0;
                    out[idx + 1] = 0;
                    out[idx + 2] = 0;
                    out[idx + 3] = 0xff;
                }
            }
        }
    }
}
#endif