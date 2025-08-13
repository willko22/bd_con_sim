#pragma once

// STB_TRUETYPE implementation wrapper
// Include this file only once in your project to avoid multiple definitions

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Optional: Add convenience functions or types here
namespace stb {
    // Wrapper class for easier usage (optional)
    class TrueTypeFont {
    public:
        stbtt_fontinfo font_info;
        unsigned char* font_data;
        
        TrueTypeFont() : font_data(nullptr) {}
        
        ~TrueTypeFont() {
            if (font_data) {
                delete[] font_data;
            }
        }
        
        bool LoadFromFile(const char* filename);
        bool LoadFromMemory(unsigned char* data, size_t size);
        float GetPixelHeightScale(float pixel_height);
        void GetBakedQuad(int char_index, float pixel_height, float* xpos, float* ypos, stbtt_aligned_quad* quad);
    };
}
