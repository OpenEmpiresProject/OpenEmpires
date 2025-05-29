#include "Frame.h"

#include <functional>
#include <iostream>

using namespace drs;
using namespace std;

#pragma pack(push, 1)
struct Outline {
  uint16_t leftSpace;
  uint16_t rightSpace;
};

struct RowCmdOffset {
  uint32_t offset;
};
#pragma pack(pop)

void decodeSLPRow(const uint8_t* cmdStream, int width, std::vector<uint32_t>& outPixels,
                  const std::function<uint32_t(uint8_t index)>& paletteLookup,
                  const std::function<uint32_t(uint8_t index)>& playerColorLookup,
                  const Outline& outlineEntry);

void Frame::load(const SLPFrameInfo& fi, std::span<const uint8_t> data)
{
    auto paletteLookup = [](uint8_t index) -> uint32_t {
        if (index == 0)
        {
            return index;
        }
        
        // Convert palette index to RGBA (dummy values for now)
        return 0xFF000000 | (index << 16) | (index << 8) | index;
    };

    auto playerColorLookup = [](uint8_t index) -> uint32_t {
        // Dummy: use magenta-tinted version
        return 0xFFFF00FF;
    };

// Bounds check on frame internals
    if (fi.width <= 0 || fi.height <= 0 || fi.width > MAX_IMAGE_SIZE || fi.height > MAX_IMAGE_SIZE) {
        throw std::runtime_error("Frame size is out of bounds.");
    }

    std::cout<< fi.width << ","<< fi.height <<"; "<< fi.hotspot_x<<","<< fi.hotspot_y << std::endl;

    // Outline Table
    size_t outlineTableSize = fi.height * sizeof(Outline);
    if (fi.outlineTableOffset + outlineTableSize > data.size()) {
        throw std::runtime_error("Outline table out of bounds.");
    }

    const Outline* outlineTable = reinterpret_cast<const Outline*>(data.data() + fi.outlineTableOffset);

    // Command Offset Table
    size_t cmdOffsetTableSize = fi.height * sizeof(RowCmdOffset);
    if (fi.cmdTableOffset + cmdOffsetTableSize > data.size()) {
        throw std::runtime_error("Cmd offset table out of bounds.");
    }

    // create double array with fi.width and fi.height and init m_image span of span
    
    
    // int frameInfoEnd = frameInfosOffset + sizeof(SLPFrameInfo) * frameCount;
    // int outlineStart = sizeof(SLPHeader) + frameCount * sizeof(SLPFrameInfo);
    // int cmdOffset2nd = outlineStart + sizeof(Outline) * frameCount;

    const RowCmdOffset* cmdOffsets = reinterpret_cast<const RowCmdOffset*>(data.data() + fi.cmdTableOffset);

    // Each row's pixel command data is located at: base + cmdOffsets[row].offset
    // Actual command decoding would happen here
    for (int row = 0; row < fi.height; ++row) {
        uint32_t rowCmdOffset = cmdOffsets[row].offset;
        if (rowCmdOffset >= data.size()) {
            throw std::runtime_error("Row command offset out of bounds.");
        }

        const uint8_t* rowCmds = data.data() + rowCmdOffset;

        std::vector<uint32_t> rowPixels;
        decodeSLPRow(rowCmds, fi.width, rowPixels, paletteLookup, playerColorLookup, outlineTable[row]);
        m_image.push_back(rowPixels);
    }

    for (size_t i = 0; i < fi.height; i++)
    {
        for (size_t j = 0; j < fi.width; j++)
        {
            std::cout<< (m_image[i][j] != 0 ? "X": " ");
        }
        std::cout<< std::endl;
    }
    
    int iii = 0;
    // TODO: Store parsed frame in a frame list/vector in SLPFile
}


void decodeSLPRow(const uint8_t* cmdStream, int width, std::vector<uint32_t>& outPixels,
                  const std::function<uint32_t(uint8_t index)>& paletteLookup,
                  const std::function<uint32_t(uint8_t index)>& playerColorLookup,
                    const Outline& outlineEntry) {
    size_t i = 0;

    if (outlineEntry.leftSpace == 0x8000 ||  outlineEntry.rightSpace == 0x8000)
    {
        for (size_t i = 0; i < width; i++)
        {
            outPixels.push_back(0x00000000);
        }
        return;
    }
    
    for (size_t i = 0; i < outlineEntry.leftSpace; i++)
    {
        outPixels.push_back(0x00000000);
    }

    while (true) {
        uint8_t opcode = cmdStream[i];
        uint8_t lowerNibble = opcode & 0x0F; // Left 4 bits
        uint8_t higherNibble = opcode & 0xF0; // Right 4 bits
        uint8_t lowestCrumb = opcode & 0x03; // Right most 2 bits

        if (lowerNibble == 0x0F)
        {
            break;
        }
        else if (lowestCrumb == 0x00) // Lesser draw
        {
            // draw the following bytes as palette colors
            int pixelCount = opcode >> 2;
            for (int j = 0; j < pixelCount; ++j)
            {
                uint32_t color = paletteLookup(cmdStream[++i]);
                outPixels.push_back(color);
            }
        }
        else if (lowestCrumb == 0x01) // Lesser skip
        {
            // draw 'count' transparent pixels
            // count = cmd >> 2; if count == 0: count = nextbyte

            int pixelCount = opcode >> 2;
            if (pixelCount == 0)
            {
                pixelCount = cmdStream[++i];
            }

            for (int j = 0; j < pixelCount; ++j)
            {
                // TODO: might not be able to use 0 for transparent
                uint32_t color = paletteLookup(0);
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x02) // Greater draw
        {
            // draw (higherNibble << 4 + nextbyte) following palette colors
            int pixelCount = int(higherNibble << 4) + cmdStream[++i];

            for (int j = 0; j < pixelCount; ++j)
            {
                uint32_t color = paletteLookup(cmdStream[++i]);
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x03) // Greater skip
        {
            // draw (higher_nibble << 4 + nextbyte)
            // transparent pixels

            int pixelCount = int(higherNibble << 4) + cmdStream[++i];
            for (int j = 0; j < pixelCount; ++j)
            {
                // TODO: might not be able to use 0 for transparent
                uint32_t color = paletteLookup(0);
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x06) // Player color
        {
            // we have to draw the player color for cmd>>4 times,
            // or if that is 0, as often as the next byte says.

            int pixelCount = opcode >> 4;
            if (pixelCount == 0)
            {
                pixelCount = cmdStream[++i];
            }
            for (int j = 0; j < pixelCount; ++j)
            {
                uint32_t color = paletteLookup(cmdStream[++i]);
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x07) // Fill
        {
            // draw 'count' pixels with color of next byte

            int pixelCount = opcode >> 4;
            if (pixelCount == 0)
            {
                pixelCount = cmdStream[++i];
            }
            uint32_t color = paletteLookup(cmdStream[++i]);

            for (int j = 0; j < pixelCount; ++j)
            {
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x0A) // Fill player folow
        {
            // draw the player color for 'count' times

            int pixelCount = opcode >> 4;
            if (pixelCount == 0)
            {
                pixelCount = cmdStream[++i];
            }
            uint32_t color = paletteLookup(cmdStream[++i]);

            for (int j = 0; j < pixelCount; ++j)
            {
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x0B) // Shadow
        {
            // draw shadow pixel for 'count' times
            
            int pixelCount = opcode >> 4;
            if (pixelCount == 0)
            {
                pixelCount = cmdStream[++i];
            }
            uint32_t color = paletteLookup(cmdStream[++i]);

            for (int j = 0; j < pixelCount; ++j)
            {
                outPixels.push_back(color);
            }
        }
        else if (lowerNibble == 0x0E) // Extended commands
        {
            if (higherNibble == 0x40 || higherNibble == 0x60)
            {
                uint32_t color = paletteLookup(0);
                outPixels.push_back(color);
            }
            else if (higherNibble == 0x50 || higherNibble == 0x70)
            {
                uint32_t color = paletteLookup(0);
                int pixelCount = cmdStream[++i];
                for (int j = 0; j < pixelCount; ++j)
                {
                    outPixels.push_back(color);
                }
            }
        }
        else
        {
            throw std::runtime_error("Unknown opcode in SLP stream.");
        }
        ++i;
        
        
        


        // if (opcode >= 0x00 && opcode <= 0x0F) {
        //     // Transparent run
        //     int count = opcode;
        //     for (int j = 0; j < count && x < width; ++j) {
        //         outPixels.push_back(0x00000000); // Transparent
        //         x++;
        //     }

        // } else if (opcode >= 0x10 && opcode <= 0x1F) {
        //     // Player color fill
        //     int count = opcode & 0x0F;
        //     uint8_t colorIndex = cmdStream[i++];
        //     for (int j = 0; j < count && x < width; ++j) {
        //         outPixels.push_back(playerColorLookup(colorIndex));
        //         x++;
        //     }

        // } else if (opcode >= 0x20 && opcode <= 0x3F) {
        //     // Raw color run
        //     int count = opcode & 0x1F;
        //     for (int j = 0; j < count && x < width; ++j) {
        //         uint8_t index = cmdStream[i++];
        //         outPixels.push_back(paletteLookup(index));
        //         x++;
        //     }

        // } else if (opcode >= 0x40 && opcode <= 0x4F) {
        //     // Shadow fill
        //     int count = opcode & 0x0F;
        //     for (int j = 0; j < count && x < width; ++j) {
        //         outPixels.push_back(0xFF000000); // Shadow = opaque black
        //         x++;
        //     }

        // } else if (opcode >= 0xF0) {
        //     // End-of-row control codes
        //     if (opcode == 0xF0) {
        //         // End of row
        //         break;
        //     } else {
        //         throw std::runtime_error("Unhandled control opcode: 0x" + std::to_string(opcode));
        //     }

        // } else {
        //     throw std::runtime_error("Unknown opcode in SLP stream.");
        // }
    }

    for (size_t i = 0; i < outlineEntry.rightSpace; i++)
    {
        outPixels.push_back(0x00000000);
    }

    // Fill remaining width with transparent pixels if any
    // while (x < width) {
    //     outPixels.push_back(0x00000000);
    //     x++;
    // }
}