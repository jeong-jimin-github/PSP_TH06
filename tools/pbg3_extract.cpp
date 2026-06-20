#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "FileSystem.hpp"
#include "pbg3/Pbg3Archive.hpp"

namespace FileSystem
{
FILE *FopenUTF8(const char *filename, const char *mode)
{
    return std::fopen(filename, mode);
}
} // namespace FileSystem

u32 g_LastFileSize = 0;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::fprintf(stderr, "usage: %s ARCHIVE.DAT OUTPUT_DIRECTORY\n", argv[0]);
        return 2;
    }

    Pbg3Archive archive;
    if (!archive.Load(argv[1]))
    {
        std::fprintf(stderr, "failed to load %s\n", argv[1]);
        return 1;
    }

    std::filesystem::create_directories(argv[2]);
    int failures = 0;
    for (u32 entryIdx = 0; entryIdx < archive.GetEntryCount(); ++entryIdx)
    {
        const char *entryName = archive.GetEntryName(entryIdx);
        u8 *data = archive.ReadDecompressEntry(entryIdx, entryName);
        if (data == NULL)
        {
            std::fprintf(stderr, "failed to extract %s\n", entryName);
            ++failures;
            continue;
        }

        const std::filesystem::path outputPath = std::filesystem::path(argv[2]) / entryName;
        std::filesystem::create_directories(outputPath.parent_path());
        FILE *output = std::fopen(outputPath.string().c_str(), "wb");
        const u32 size = archive.GetEntrySize(entryIdx);
        if (output == NULL || (size > 0 && std::fwrite(data, 1, size, output) != size))
        {
            std::fprintf(stderr, "failed to write %s\n", outputPath.string().c_str());
            ++failures;
        }
        if (output != NULL)
        {
            std::fclose(output);
        }
        std::free(data);
    }

    return failures == 0 ? 0 : 1;
}
