#ifndef IMAGE_HPP
#define IMAGE_HPP
#include <cstring>
#include "glm/glm.hpp"
#ifndef MAX_BYTE
#define MAX_BYTE 0xff
#endif
#ifndef ROW_COLUMN_TO_INDEX
#define ROW_COLUMN_TO_INDEX(ROW_INDEX, COLUMN_INDEX, COLUMN)((ROW_INDEX) * (COLUMN) + (COLUMN_INDEX))
#endif
namespace ananImage{
    // static inline void copy(const void *source, void *destination, uint32_t row, uint32_t column, const glm::uvec2&cutSzie, uint32_t srcWidth){
    //     //图片必须是一个像素4个字节或者rgba
    //     //如果要截取第2行的图片，需要跳过 row * 目标图片的高度 * 原图宽度
    //     const uint32_t uiLineSize = cutSzie.x * 4;
    //     const uint32_t uiSourceLineSize = srcWidth * 4;
    //     const uint32_t imageOffsetStart = row * cutSzie.y * uiSourceLineSize + column * uiLineSize;
    //     for (size_t dataOffset = 0; dataOffset < cutSzie.y; ++dataOffset) {
    //         memcpy((char *)destination + dataOffset * uiLineSize, (char *)source + imageOffsetStart + dataOffset * uiSourceLineSize, uiLineSize);
    //     }
    // }
    // static inline void copy(const stbi_uc *source, stbi_uc *destination, const glm::uvec2&srcSize, const glm::uvec2&dstSize, const glm::uvec2&destOffset){
    //     const uint32_t dstLineWidth = dstSize.x * 4, srcLineWidth = srcSize.x * 4;
    //     for (size_t dataOffset = 0; dataOffset < srcSize.y; ++dataOffset) {
    //         //TODO:还需要加入偏移
    //         memcpy(destination + dataOffset * dstLineWidth, source + dataOffset * srcLineWidth, srcLineWidth);
    //         // memcpy(destination + destOffset.y + dataOffset * dstLineWidth, source + destOffset.x * 4 + dataOffset * srcLineWidth, srcLineWidth);
    //     }
    // }
    // static inline void calculateImageSize(const glm::uvec2& extent, const glm::uvec2& size, glm::uvec2& newSize){    
    //     float widthRatio = static_cast<float>(extent.x - 1) / static_cast<float>(size.x);
    //     float heightRatio = static_cast<float>(extent.y - 1) / static_cast<float>(size.y);
    //     float scale = std::min(widthRatio, heightRatio);
    //     newSize.x = static_cast<uint32_t>(std::round(size.x * scale));
    //     newSize.y = static_cast<uint32_t>(std::round(size.y * scale));
    //     newSize.x = std::min(newSize.x, extent.x > 1 ? extent.x - 1 : 1u);
    //     newSize.y = std::min(newSize.y, extent.y > 1 ? extent.y - 1 : 1u);
    //     newSize.x = std::max(newSize.x, 1u);
    //     newSize.y = std::max(newSize.y, 1u);
    //     assert(newSize.x < extent.x && newSize.y < extent.y);
    // }
	static inline glm::uvec2 calculateRotatedSize(float angle, const glm::uvec2& size) {
	    float angle_rad = glm::radians(angle);
	    float cos_angle = std::cos(angle_rad), sin_angle = std::sin(angle_rad);
    
	    glm::vec2 corners[4] = {
	        glm::vec2(0, 0),
	        glm::vec2(size.x, 0),
		    glm::vec2(0, size.y),
	        glm::vec2(size.x, size.y)
		};
    
	    glm::vec2 center(size.x * 0.5f, size.y * 0.5f);
    
	    glm::vec2 min = glm::vec2(std::numeric_limits<float>::max()), max = glm::vec2(std::numeric_limits<float>::lowest());
    
	    for (const auto& corner : corners) {
	        glm::vec2 rotated = glm::vec2(
		        (corner.x - center.x) * cos_angle - (corner.y - center.y) * sin_angle + center.x,
	            (corner.x - center.x) * sin_angle + (corner.y - center.y) * cos_angle + center.y
		    );
	        min.x = std::min(min.x, rotated.x);
	        max.x = std::max(max.x, rotated.x);
	        min.y = std::min(min.y, rotated.y);
			max.y = std::max(max.y, rotated.y);
		}
            
		return glm::uvec2(std::ceil(max.x - min.x), std::ceil(max.y - min.y));
	}
    glm::vec2 calculateImageSize(const glm::vec2& extent, const glm::vec2& imageSize, uint32_t imageCount) {
        if (extent.x <= 0 || extent.y <= 0 || imageSize.x <= 0 || imageSize.y <= 0 || imageCount == 0) {
            return glm::vec2(0.0f);
        }
        else if (imageCount == 1) {
            float widthRatio = extent.x / imageSize.x;
            float heightRatio = extent.y / imageSize.y;
            float scale = glm::min(widthRatio, heightRatio);
            return imageSize * scale;
        }

        glm::vec2 bestSize(0.0f);
        float maxArea = 0.0f;

        for (uint32_t rows = 1; rows <= imageCount; ++rows) {
            uint32_t cols = (imageCount + rows - 1) / rows; // 向上取整
            
            float cellWidth = extent.x / cols;
            float cellHeight = extent.y / rows;
            
            // 计算在此单元格内能缩放的最大尺寸（保持宽高比）
            float widthRatio = cellWidth / imageSize.x;
            float heightRatio = cellHeight / imageSize.y;
            float scale = glm::min(widthRatio, heightRatio);
            
            glm::vec2 scaledSize = imageSize * scale;
            float area = scaledSize.x * scaledSize.y;
            
            // 选择能获得最大图片面积的布局
            if (area > maxArea) {
                maxArea = area;
                bestSize = scaledSize;
            }
            
            // 对称性：也尝试交换行列（当可能时）
            uint32_t altRows = cols;
            uint32_t altCols = rows;
            if (altRows * altCols >= imageCount && altRows <= imageCount) {
                float altCellWidth = extent.x / altCols;
                float altCellHeight = extent.y / altRows;
                float altWidthRatio = altCellWidth / imageSize.x;
                float altHeightRatio = altCellHeight / imageSize.y;
                float altScale = glm::min(altWidthRatio, altHeightRatio);
                
                glm::vec2 altScaledSize = imageSize * altScale;
                float altArea = altScaledSize.x * altScaledSize.y;
                
                if (altArea > maxArea) {
                    maxArea = altArea;
                    bestSize = altScaledSize;
                }
            }
        }

        return bestSize;
    }
    static inline void removeInvalidPixel(unsigned char *data, const glm::uvec2&size, const glm::uvec3&pixel){
        for (uint32_t i = 0; i < size.x * size.y; ++i){
            if(data[i * 4] == pixel.x && data[i * 4 + 1] == pixel.y && data[i * 4 + 2] == pixel.z){
                data[i * 4 + 3] = 0;
            }
        }
    }
    static inline void convertFontToRGBA(const unsigned char *fontData, const glm::uvec2 &size, unsigned char *data){
        uint32_t width = size.x;
        uint32_t height = size.y;
        uint32_t totalPixels = width * height;
        memset(data, 0, totalPixels * 4);
        for (uint32_t i = 0; i < totalPixels; ++i) {
            uint32_t dstIndex = i * 4;
            data[dstIndex] = fontData[i];
            // data[dstIndex + 3] = MAX_BYTE;
        }
    }
    static inline void copy(const unsigned char *source, unsigned char *destination, const glm::uvec2&cutSize, const glm::uvec2& srcSize, const glm::uvec2& dstSize, const glm::uvec2& srcOffset = glm::uvec2(0), const glm::uvec2& destOffset = glm::uvec2(0)) {
        const uint32_t channels = 4;
        const uint32_t srcImageSIze = srcSize.x * srcSize.y * channels, dstImageSize = dstSize.x * dstSize.y * channels;
        if (srcOffset.x + cutSize.x > srcSize.x || srcOffset.y + cutSize.y > srcSize.y ||
            destOffset.x + cutSize.x > dstSize.x || destOffset.y + cutSize.y > dstSize.y) {
            printf("Error in %s: Copy region out of bounds.\n", __FUNCTION__);
            return;
        }
        for (uint32_t y = 0; y < cutSize.y; ++y) {
            for (uint32_t x = 0; x < cutSize.x; ++x) {
                const uint32_t srcIndex = ROW_COLUMN_TO_INDEX(y + srcOffset.y, x + srcOffset.x, srcSize.x) * channels;
                const uint32_t dstIndex = ROW_COLUMN_TO_INDEX(destOffset.y + y, destOffset.x + x, dstSize.x) * channels;
                // if (srcIndex + 3 >= srcImageSIze || dstIndex + 3 >= dstImageSize){
                //     printf("in function %s:srcIndex(%d) + 3 >= srcImageSIze(%d) || dstIndex(%d) + 3 >= dstImageSize(%d)\n", __FUNCTION__, srcIndex, srcImageSIze, dstIndex, dstImageSize);
                //     break;
                // }
                if (source[srcIndex + 3]) {
                    destination[dstIndex]     = source[srcIndex];
                    destination[dstIndex + 1] = source[srcIndex + 1];
                    destination[dstIndex + 2] = source[srcIndex + 2];
                    destination[dstIndex + 3] = MAX_BYTE;
                }
            }
        }
    }

	static inline glm::vec4 samplePixel(const unsigned char *data, const glm::uvec2& size, const glm::vec2& currentPixel) {
	    const uint32_t m_channels = 4;
	    int32_t x0 = glm::clamp(static_cast<int32_t>(std::floor(currentPixel.x)), 0, static_cast<int>(size.x) - 1);
	    int32_t y0 = glm::clamp(static_cast<int32_t>(std::floor(currentPixel.y)), 0, static_cast<int>(size.y) - 1);
	    int32_t x1 = glm::clamp(x0 + 1, 0, static_cast<int>(size.x) - 1);
	    int32_t y1 = glm::clamp(y0 + 1, 0, static_cast<int>(size.y) - 1);
    
	    glm::vec2 d = glm::vec2(currentPixel.x - x0, currentPixel.y - y0);
    
	    auto getPixel = [&](int px, int py) -> glm::vec4 {
	        int index = (py * size.x + px) * m_channels;
	        glm::vec4 result(0.0f);
	        for (int c = 0; c < 4; ++c) {
		        if (c < m_channels) {
	                result[c] = data[index + c] / 255.0f;
	            }
	        }
		    if (m_channels < 4) result.a = 1.0f;
	        return result;
	    };
    
	    glm::vec4 p00 = getPixel(x0, y0);
	    glm::vec4 p10 = getPixel(x1, y0);
	    glm::vec4 p01 = getPixel(x0, y1);
	    glm::vec4 p11 = getPixel(x1, y1);
    
	    glm::vec4 top = glm::mix(p00, p10, d.x);
	    glm::vec4 bottom = glm::mix(p01, p11, d.x);
	    return glm::mix(top, bottom, d.y);
	} 
    static inline bool rotate(const unsigned char *data, const glm::uvec2& size, float angle, unsigned char *out, const glm::uvec2& outSize) {
        const uint32_t m_channels = 4;
        glm::vec2 old_center(size.x * 0.5f, size.y * 0.5f);
        glm::vec2 new_center(outSize.x * 0.5f, outSize.y * 0.5f);
        
        float angle_rad = glm::radians(angle);
        float cos_angle = std::cos(angle_rad);
        float sin_angle = std::sin(angle_rad);
        
        for (int32_t y = 0; y < outSize.y; ++y) {
            for (int32_t x = 0; x < outSize.x; ++x) {
                glm::vec2 translated = glm::vec2(x - new_center.x, y - new_center.y);

                glm::vec2 rotated = glm::vec2(
                    translated.x * cos_angle + translated.y * sin_angle,
                    -translated.x * sin_angle + translated.y * cos_angle
                );
                
                glm::vec2 src = rotated + old_center;

                int32_t new_index = (y * outSize.x + x) * m_channels;
                
                if (src.x >= 0 && src.x < size.x && src.y >= 0 && src.y < size.y) {
                    glm::vec4 pixel = samplePixel(data, size, src);
                    
                    for (int32_t c = 0; c < m_channels; ++c) {
                        out[new_index + c] = static_cast<unsigned char>(glm::clamp(pixel[c] * 255.0f, 0.0f, 255.0f));
                    }
                } else {
                    for (int32_t c = 0; c < m_channels; ++c) {
                        out[new_index + c] = 0;
                    }
                }
            }
        }
        return true;
    }
};
#endif
