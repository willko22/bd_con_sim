#include "utils/stb_truetype_wrapper.h"
#include <fstream>
#include <iostream>

namespace stb {
    bool TrueTypeFont::LoadFromFile(const char* filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Failed to open font file: " << filename << std::endl;
            return false;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        font_data = new unsigned char[size];
        if (!file.read(reinterpret_cast<char*>(font_data), size)) {
            std::cerr << "Failed to read font file: " << filename << std::endl;
            delete[] font_data;
            font_data = nullptr;
            return false;
        }
        
        if (!stbtt_InitFont(&font_info, font_data, 0)) {
            std::cerr << "Failed to initialize font from file: " << filename << std::endl;
            delete[] font_data;
            font_data = nullptr;
            return false;
        }
        
        return true;
    }
    
    bool TrueTypeFont::LoadFromMemory(unsigned char* data, size_t size) {
        font_data = new unsigned char[size];
        memcpy(font_data, data, size);
        
        if (!stbtt_InitFont(&font_info, font_data, 0)) {
            std::cerr << "Failed to initialize font from memory" << std::endl;
            delete[] font_data;
            font_data = nullptr;
            return false;
        }
        
        return true;
    }
    
    float TrueTypeFont::GetPixelHeightScale(float pixel_height) {
        return stbtt_ScaleForPixelHeight(&font_info, pixel_height);
    }
    
    void TrueTypeFont::GetBakedQuad(int char_index, float pixel_height, float* xpos, float* ypos, stbtt_aligned_quad* quad) {
        // This is a simplified version - for full functionality you'd need to implement bitmap generation
        // For now, just provide the basic structure
        float scale = GetPixelHeightScale(pixel_height);
        
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font_info, char_index, &advance, &lsb);
        
        // This would typically involve generating bitmap data and creating quads
        // For demonstration purposes, we're just showing the structure
        quad->x0 = *xpos;
        quad->y0 = *ypos;
        quad->x1 = *xpos + advance * scale;
        quad->y1 = *ypos + pixel_height;
        
        *xpos += advance * scale;
    }
}
