#pragma once

#include "inttypes.hpp"
#include "pbg3/Pbg3Parser.hpp"

struct Pbg3Entry
{
    u32 unk1;
    u32 unk2;
    u32 uncompressedSize;
    u32 dataOffset;
    u32 checksum;
    char filename[256];
};

class Pbg3Archive
{
  public:
    Pbg3Archive();
    ~Pbg3Archive();

    i32 Release();

    i32 Load(const char *path);
    i32 ParseHeader();
    i32 FindEntry(const char *path);
    u32 GetEntryCount() const;
    const char *GetEntryName(u32 entryIdx) const;
    u32 GetEntrySize(u32 entryIdx);
    u8 *ReadEntryRaw(u32 *outSize, u32 *outChecksum, i32 entryIdx);
    u8 *ReadDecompressEntry(u32 entryIdx, const char *filename);
    u8 *ReadExtractedEntry(u32 entryIdx);

  private:
    Pbg3Parser *parser;
    void *unk;
    u32 numOfEntries;
    u32 fileTableOffset;
    Pbg3Entry *entries;
    char sidecarDirectory[64];
};

extern Pbg3Archive **g_Pbg3Archives;
