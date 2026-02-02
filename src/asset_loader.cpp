#include "asset_loader.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

namespace SMStrikers {

namespace {

enum GXTextureFormat {
    GXTex_RGB565 = 0,
    GXTex_RGB5A3 = 1,
    GXTex_CMPR = 2,
    GXTex_RGBA8 = 3,
    GXTex_I8 = 4,
    GXTex_I4 = 5,
    GXTex_A8 = 6,
    GXTex_IA8 = 7,
    GXTex_CI8 = 8
};

uint32_t readU32BE(const std::vector<uint8_t>& data, size_t offset) {
    return (static_cast<uint32_t>(data[offset]) << 24) |
           (static_cast<uint32_t>(data[offset + 1]) << 16) |
           (static_cast<uint32_t>(data[offset + 2]) << 8) |
           static_cast<uint32_t>(data[offset + 3]);
}


uint16_t readU16BE(const std::vector<uint8_t>& data, size_t offset) {
    return static_cast<uint16_t>((data[offset] << 8) | data[offset + 1]);
}

uint16_t readU16BE(const uint8_t* data) {
    return static_cast<uint16_t>((data[0] << 8) | data[1]);
}

uint32_t readU32BE(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8) |
           static_cast<uint32_t>(data[3]);
}

struct TileInfo {
    int tileW;
    int tileH;
    int bytesPerTile;
};

size_t gcTextureSize(uint32_t format, int width, int height, int levels) {
    size_t total = 0;
    for (;;) {
        int rowBytes;
        if (format == GXTex_CMPR) {
            rowBytes = width >> 1;
        } else if (format == GXTex_RGBA8) {
            rowBytes = width << 2;
        } else if (format == GXTex_A8 || format == GXTex_CI8 || format == GXTex_I8) {
            rowBytes = width;
        } else {
            rowBytes = width << 1;
        }

        total += static_cast<size_t>(height) * static_cast<size_t>(rowBytes);

        levels -= 1;
        if (levels == 0) {
            break;
        }

        width >>= 1;
        height >>= 1;
    }

    return total;
}

uint8_t expand4To8(uint8_t value) {
    return static_cast<uint8_t>((value << 4) | value);
}

uint8_t expand5To8(uint8_t value) {
    return static_cast<uint8_t>((value * 255 + 15) / 31);
}

uint8_t expand6To8(uint8_t value) {
    return static_cast<uint8_t>((value * 255 + 31) / 63);
}

uint8_t expand3To8(uint8_t value) {
    return static_cast<uint8_t>((value * 255 + 3) / 7);
}

void decodeRGB565(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = expand5To8(static_cast<uint8_t>((value >> 11) & 0x1F));
    g = expand6To8(static_cast<uint8_t>((value >> 5) & 0x3F));
    b = expand5To8(static_cast<uint8_t>(value & 0x1F));
}

void decodeRGB5A3(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    if (value & 0x8000) {
        r = expand5To8(static_cast<uint8_t>((value >> 10) & 0x1F));
        g = expand5To8(static_cast<uint8_t>((value >> 5) & 0x1F));
        b = expand5To8(static_cast<uint8_t>(value & 0x1F));
        a = 255;
    } else {
        a = expand3To8(static_cast<uint8_t>((value >> 12) & 0x7));
        r = expand4To8(static_cast<uint8_t>((value >> 8) & 0xF));
        g = expand4To8(static_cast<uint8_t>((value >> 4) & 0xF));
        b = expand4To8(static_cast<uint8_t>(value & 0xF));
    }
}

void writePixel(std::vector<uint8_t>& out, int width, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    size_t index = static_cast<size_t>((y * width + x) * 4);
    out[index] = r;
    out[index + 1] = g;
    out[index + 2] = b;
    out[index + 3] = a;
}

void decodeI4(const uint8_t* data, int width, int height, std::vector<uint8_t>& out) {
    TileInfo info{8, 8, 32};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            for (int p = 0; p < info.tileW * info.tileH; ++p) {
                uint8_t byte = tile[p / 2];
                uint8_t nibble = (p % 2 == 0) ? (byte >> 4) : (byte & 0xF);
                uint8_t intensity = expand4To8(nibble);
                int x = (p % info.tileW) + tx * info.tileW;
                int y = (p / info.tileW) + ty * info.tileH;
                if (x < width && y < height) {
                    writePixel(out, width, x, y, intensity, intensity, intensity, 255);
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

void decodeI8(const uint8_t* data, int width, int height, std::vector<uint8_t>& out, bool alphaOnly) {
    TileInfo info{8, 4, 32};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            for (int p = 0; p < info.tileW * info.tileH; ++p) {
                uint8_t intensity = tile[p];
                int x = (p % info.tileW) + tx * info.tileW;
                int y = (p / info.tileW) + ty * info.tileH;
                if (x < width && y < height) {
                    uint8_t r = alphaOnly ? 255 : intensity;
                    uint8_t g = alphaOnly ? 255 : intensity;
                    uint8_t b = alphaOnly ? 255 : intensity;
                    uint8_t a = alphaOnly ? intensity : 255;
                    writePixel(out, width, x, y, r, g, b, a);
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

void decodeIA8(const uint8_t* data, int width, int height, std::vector<uint8_t>& out) {
    TileInfo info{4, 4, 32};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            for (int p = 0; p < info.tileW * info.tileH; ++p) {
                uint8_t intensity = tile[p * 2];
                uint8_t alpha = tile[p * 2 + 1];
                int x = (p % info.tileW) + tx * info.tileW;
                int y = (p / info.tileW) + ty * info.tileH;
                if (x < width && y < height) {
                    writePixel(out, width, x, y, intensity, intensity, intensity, alpha);
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

void decodeRGB16(const uint8_t* data, int width, int height, std::vector<uint8_t>& out, bool useRGB5A3) {
    TileInfo info{4, 4, 32};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            for (int p = 0; p < info.tileW * info.tileH; ++p) {
                uint16_t value = readU16BE(tile + p * 2);
                uint8_t r = 0, g = 0, b = 0, a = 255;
                if (useRGB5A3) {
                    decodeRGB5A3(value, r, g, b, a);
                } else {
                    decodeRGB565(value, r, g, b);
                }
                int x = (p % info.tileW) + tx * info.tileW;
                int y = (p / info.tileW) + ty * info.tileH;
                if (x < width && y < height) {
                    writePixel(out, width, x, y, r, g, b, a);
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

void decodeRGBA8(const uint8_t* data, int width, int height, std::vector<uint8_t>& out) {
    TileInfo info{4, 4, 64};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            const uint8_t* ar = tile;
            const uint8_t* gb = tile + 32;
            for (int p = 0; p < info.tileW * info.tileH; ++p) {
                uint8_t a = ar[p * 2];
                uint8_t r = ar[p * 2 + 1];
                uint8_t g = gb[p * 2];
                uint8_t b = gb[p * 2 + 1];
                int x = (p % info.tileW) + tx * info.tileW;
                int y = (p / info.tileW) + ty * info.tileH;
                if (x < width && y < height) {
                    writePixel(out, width, x, y, r, g, b, a);
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

void decodeCI8(const uint8_t* data, int width, int height, const std::vector<uint16_t>& palette, std::vector<uint8_t>& out) {
    TileInfo info{8, 4, 32};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            for (int p = 0; p < info.tileW * info.tileH; ++p) {
                uint8_t index = tile[p];
                uint8_t r = 0, g = 0, b = 0, a = 255;
                if (index < palette.size()) {
                    decodeRGB5A3(palette[index], r, g, b, a);
                }
                int x = (p % info.tileW) + tx * info.tileW;
                int y = (p / info.tileW) + ty * info.tileH;
                if (x < width && y < height) {
                    writePixel(out, width, x, y, r, g, b, a);
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

void decodeCMPRBlock(const uint8_t* block, uint8_t colors[4][4]) {
    uint16_t color0 = readU16BE(block);
    uint16_t color1 = readU16BE(block + 2);
    uint8_t r0, g0, b0;
    uint8_t r1, g1, b1;
    decodeRGB565(color0, r0, g0, b0);
    decodeRGB565(color1, r1, g1, b1);

    colors[0][0] = r0;
    colors[0][1] = g0;
    colors[0][2] = b0;
    colors[0][3] = 255;

    colors[1][0] = r1;
    colors[1][1] = g1;
    colors[1][2] = b1;
    colors[1][3] = 255;

    if (color0 > color1) {
        colors[2][0] = static_cast<uint8_t>((2 * r0 + r1) / 3);
        colors[2][1] = static_cast<uint8_t>((2 * g0 + g1) / 3);
        colors[2][2] = static_cast<uint8_t>((2 * b0 + b1) / 3);
        colors[2][3] = 255;

        colors[3][0] = static_cast<uint8_t>((r0 + 2 * r1) / 3);
        colors[3][1] = static_cast<uint8_t>((g0 + 2 * g1) / 3);
        colors[3][2] = static_cast<uint8_t>((b0 + 2 * b1) / 3);
        colors[3][3] = 255;
    } else {
        colors[2][0] = static_cast<uint8_t>((r0 + r1) / 2);
        colors[2][1] = static_cast<uint8_t>((g0 + g1) / 2);
        colors[2][2] = static_cast<uint8_t>((b0 + b1) / 2);
        colors[2][3] = 255;

        colors[3][0] = 0;
        colors[3][1] = 0;
        colors[3][2] = 0;
        colors[3][3] = 0;
    }
}

void decodeCMPR(const uint8_t* data, int width, int height, std::vector<uint8_t>& out) {
    TileInfo info{8, 8, 32};
    int tilesX = (width + info.tileW - 1) / info.tileW;
    int tilesY = (height + info.tileH - 1) / info.tileH;
    size_t offset = 0;
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            const uint8_t* tile = data + offset;
            for (int blockY = 0; blockY < 2; ++blockY) {
                for (int blockX = 0; blockX < 2; ++blockX) {
                    const uint8_t* block = tile + (blockY * 2 + blockX) * 8;
                    uint8_t colors[4][4];
                    decodeCMPRBlock(block, colors);
                    uint32_t indices = readU32BE(block + 4);
                    for (int py = 0; py < 4; ++py) {
                        for (int px = 0; px < 4; ++px) {
                            int pixelIndex = py * 4 + px;
                            int shift = 30 - (pixelIndex * 2);
                            uint8_t code = static_cast<uint8_t>((indices >> shift) & 0x3);
                            int x = tx * info.tileW + blockX * 4 + px;
                            int y = ty * info.tileH + blockY * 4 + py;
                            if (x < width && y < height) {
                                writePixel(out, width, x, y,
                                           colors[code][0],
                                           colors[code][1],
                                           colors[code][2],
                                           colors[code][3]);
                            }
                        }
                    }
                }
            }
            offset += info.bytesPerTile;
        }
    }
}

bool decodeTexture(uint32_t format, int width, int height, const uint8_t* data,
                   const std::vector<uint16_t>& palette, std::vector<uint8_t>& out) {
    out.assign(static_cast<size_t>(width) * static_cast<size_t>(height) * 4, 0);
    switch (format) {
    case GXTex_I4:
        decodeI4(data, width, height, out);
        return true;
    case GXTex_I8:
        decodeI8(data, width, height, out, false);
        return true;
    case GXTex_A8:
        decodeI8(data, width, height, out, true);
        return true;
    case GXTex_IA8:
        decodeIA8(data, width, height, out);
        return true;
    case GXTex_RGB565:
        decodeRGB16(data, width, height, out, false);
        return true;
    case GXTex_RGB5A3:
        decodeRGB16(data, width, height, out, true);
        return true;
    case GXTex_RGBA8:
        decodeRGBA8(data, width, height, out);
        return true;
    case GXTex_CI8:
        decodeCI8(data, width, height, palette, out);
        return true;
    case GXTex_CMPR:
        decodeCMPR(data, width, height, out);
        return true;
    default:
        return false;
    }
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

AssetLoadResult loadFileStats(const std::filesystem::path& path, const char* label) {
    AssetLoadResult result;
    try {
        if (!std::filesystem::exists(path)) {
            result.message = "File not found";
            return result;
        }
        if (!std::filesystem::is_regular_file(path)) {
            result.message = "Not a regular file";
            return result;
        }
        result.fileSize = std::filesystem::file_size(path);
        result.success = true;
        result.message = std::string("Loaded ") + label + " (placeholder)";
        return result;
    } catch (const std::exception& e) {
        result.message = e.what();
        return result;
    }
}

AssetLoadResult loadTextureBundle(const std::filesystem::path& path) {
    AssetLoadResult result;
    try {
        if (!std::filesystem::exists(path)) {
            result.message = "File not found";
            return result;
        }
        if (!std::filesystem::is_regular_file(path)) {
            result.message = "Not a regular file";
            return result;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            result.message = "Failed to open file";
            return result;
        }

        file.seekg(0, std::ios::end);
        std::streamoff size = file.tellg();
        if (size <= 0) {
            result.message = "Empty file";
            return result;
        }
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(static_cast<size_t>(size));
        file.read(reinterpret_cast<char*>(data.data()), size);
        if (!file) {
            result.message = "Failed to read file";
            return result;
        }

        if (data.size() < 0x20) {
            result.message = "File too small for GLT header";
            return result;
        }

        struct GltLayout {
            size_t dictOffset;
            size_t headerSize;
            size_t widthOffset;
            size_t heightOffset;
            size_t numEntriesOffset;
            bool hasNumEntries;
            const char* label;
        };

        auto parseBundle = [&](const GltLayout& layout) -> std::shared_ptr<TextureBundle> {
            uint32_t numTextures = readU32BE(data, 4);
            if (numTextures == 0 || numTextures > 10000) {
                return {};
            }
            size_t dictSize = static_cast<size_t>(numTextures) * 0x10;
            if (layout.dictOffset + dictSize > data.size()) {
                return {};
            }
            size_t textureDataOffset = layout.dictOffset + dictSize;

            auto bundle = std::make_shared<TextureBundle>();
            bundle->textures.reserve(numTextures);

            for (uint32_t i = 0; i < numTextures; ++i) {
                size_t entryOffset = layout.dictOffset + static_cast<size_t>(i) * 0x10;
                uint32_t hash = readU32BE(data, entryOffset);
                uint32_t offset = readU32BE(data, entryOffset + 4);
                uint32_t fileSize = readU32BE(data, entryOffset + 8);

                std::cout << "GLT dict[" << i << "] hash=0x" << std::hex << hash
                          << " offset=0x" << offset << " size=0x" << fileSize
                          << std::dec << "\n";

                size_t textureOffset = textureDataOffset + offset;
                if (textureOffset + layout.headerSize > data.size()) {
                    continue;
                }
                if (fileSize != 0 && textureOffset + fileSize > data.size()) {
                    continue;
                }

                uint32_t numLevels = readU32BE(data, textureOffset);
                uint32_t format = readU32BE(data, textureOffset + 4);
                uint16_t width = readU16BE(data, textureOffset + layout.widthOffset);
                uint16_t height = readU16BE(data, textureOffset + layout.heightOffset);
                uint32_t numEntries = layout.hasNumEntries ? readU32BE(data, textureOffset + layout.numEntriesOffset) : 0;

                if (numLevels == 0) {
                    continue;
                }
                if (format > GXTex_CI8 || width == 0 || height == 0 || width > 4096 || height > 4096) {
                    continue;
                }

                size_t textureDataSize = gcTextureSize(format, width, height, static_cast<int>(numLevels));
                size_t textureDataStart = textureOffset + layout.headerSize;
                size_t paletteStart = textureDataStart + textureDataSize;

                if (textureDataStart + textureDataSize > data.size()) {
                    continue;
                }

                std::vector<uint16_t> palette;
                if (numEntries > 0) {
                    palette.reserve(numEntries);
                    size_t paletteBytes = static_cast<size_t>(numEntries) * 2;
                    if (paletteStart + paletteBytes > data.size()) {
                        continue;
                    }
                    for (uint32_t p = 0; p < numEntries; ++p) {
                        palette.push_back(readU16BE(data, paletteStart + p * 2));
                    }
                }

                TextureImage image;
                image.hash = hash;
                image.width = width;
                image.height = height;
                image.format = format;
                image.numLevels = numLevels;
                image.paletteEntries = numEntries;

                if (!decodeTexture(format, width, height, data.data() + textureDataStart, palette, image.rgba)) {
                    continue;
                }

                bundle->textures.push_back(std::move(image));
            }

            if (bundle->textures.empty()) {
                return {};
            }
            return bundle;
        };

        const GltLayout layout20 {0x20, 0x20, 0x0E, 0x10, 0x14, true, "layout20"};
        auto bundle = parseBundle(layout20);
        if (bundle) {
            std::cout << "GLT parsed using " << layout20.label << ": " << bundle->textures.size() << " textures\n";
        } else {
            const GltLayout layout10 {0x10, 0x10, 0x0C, 0x0E, 0, false, "layout10"};
            bundle = parseBundle(layout10);
            if (bundle) {
                std::cout << "GLT parsed using " << layout10.label << ": " << bundle->textures.size() << " textures\n";
            }
        }

        if (!bundle) {
            result.message = "No textures decoded";
            return result;
        }

        result.success = true;
        result.fileSize = std::filesystem::file_size(path);
        result.textureBundle = bundle;
        std::ostringstream message;
        message << "Loaded " << bundle->textures.size() << " texture";
        if (bundle->textures.size() != 1) {
            message << "s";
        }
        result.message = message.str();
        return result;
    } catch (const std::exception& e) {
        result.message = e.what();
        return result;
    }
}
class GltLoader final : public IAssetLoader {
public:
    AssetLoadResult load(const std::filesystem::path& path) const override {
        return loadTextureBundle(path);
    }
    const char* name() const override { return "GLT Loader"; }
    const char* extension() const override { return ".glt"; }
};

class GlgLoader final : public IAssetLoader {
public:
    AssetLoadResult load(const std::filesystem::path& path) const override {
        return loadFileStats(path, "model bundle");
    }
    const char* name() const override { return "GLG Loader"; }
    const char* extension() const override { return ".glg"; }
};

} // namespace

AssetLoaderRegistry::AssetLoaderRegistry() {
    registerLoader(std::make_unique<GltLoader>());
    registerLoader(std::make_unique<GlgLoader>());
}

const IAssetLoader* AssetLoaderRegistry::getLoaderForExtension(const std::string& extension) const {
    std::string key = toLower(extension);
    auto it = m_loaderByExtension.find(key);
    if (it == m_loaderByExtension.end()) {
        return nullptr;
    }
    return m_loaders[it->second].get();
}

void AssetLoaderRegistry::registerLoader(std::unique_ptr<IAssetLoader> loader) {
    std::string key = toLower(loader->extension());
    m_loaderByExtension[key] = m_loaders.size();
    m_loaders.push_back(std::move(loader));
}

} // namespace SMStrikers
