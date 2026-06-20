#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

#include "FileSystem.hpp"
#include "pbg3/Pbg3Archive.hpp"

Pbg3Archive **g_Pbg3Archives;

Pbg3Archive::Pbg3Archive()
{
    this->fileTableOffset = 0;
    this->numOfEntries = 0;
    this->entries = NULL;
    this->parser = NULL;
    this->unk = NULL;
    this->sidecarDirectory[0] = '\0';
}

i32 Pbg3Archive::ParseHeader()
{
    if (this->parser->ReadMagic() != 0x33474250)
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return false;
    }

    this->numOfEntries = this->parser->ReadVarInt();
    this->fileTableOffset = this->parser->ReadVarInt();
    if (!this->parser->SeekToOffset(this->fileTableOffset))
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return false;
    }

    this->entries = new Pbg3Entry[this->numOfEntries];
    if (this->entries == NULL)
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return false;
    }

    for (u32 idx = 0; idx < this->numOfEntries; idx += 1)
    {
        this->entries[idx].unk2 = this->parser->ReadVarInt();
        this->entries[idx].unk1 = this->parser->ReadVarInt();
        this->entries[idx].checksum = this->parser->ReadVarInt();
        this->entries[idx].dataOffset = this->parser->ReadVarInt();
        this->entries[idx].uncompressedSize = this->parser->ReadVarInt();
        if (!this->parser->ReadString(this->entries[idx].filename, sizeof(this->entries[idx].filename)))
        {
            if (this->parser != NULL)
            {
                delete this->parser;
                this->parser = NULL;
            }
            if (this->entries != NULL)
            {
                delete[] this->entries;
                this->entries = NULL;
            }

            return false;
        }
    }

    return true;
}

i32 Pbg3Archive::Release()
{
    this->fileTableOffset = 0;
    this->numOfEntries = 0;
    if (this->parser != NULL)
    {
        delete this->parser;
        this->parser = NULL;
    }
    if (this->entries != NULL)
    {
        delete[] this->entries;
        this->entries = NULL;
    }
    std::free(this->unk);
    return true;
}

i32 Pbg3Archive::FindEntry(const char *path)
{
    for (u32 entryIdx = 0; entryIdx < this->numOfEntries; entryIdx += 1)
    {
        char *entryFilename = this->entries[entryIdx].filename;
        i32 res = std::strcmp(path, entryFilename);
        if (res == 0)
        {
            return entryIdx;
        }
    }
    return -1;
}

u32 Pbg3Archive::GetEntryCount() const
{
    return this->numOfEntries;
}

const char *Pbg3Archive::GetEntryName(u32 entryIdx) const
{
    return entryIdx < this->numOfEntries ? this->entries[entryIdx].filename : NULL;
}

u32 Pbg3Archive::GetEntrySize(u32 entryIdx)
{
    if (entryIdx >= this->numOfEntries)
    {
        return 0;
    }

    return this->entries[entryIdx].uncompressedSize;
}

u8 *Pbg3Archive::ReadEntryRaw(u32 *outSize, u32 *outChecksum, i32 entryIdx)
{
    if (this->parser == NULL)
    {
        return NULL;
    }

    if (entryIdx >= this->numOfEntries)
        return NULL;

    if (outSize == NULL)
        return NULL;

    if (outChecksum == NULL)
        return NULL;

    if (!this->parser->SeekToOffset(this->entries[entryIdx].dataOffset))
        return NULL;

    u32 size;
    if (entryIdx == this->numOfEntries - 1)
    {
        size = this->fileTableOffset - this->entries[entryIdx].dataOffset;
    }
    else
    {
        size = this->entries[entryIdx + 1].dataOffset - this->entries[entryIdx].dataOffset;
    }

    u8 *data = (u8 *)malloc(size);
    if (data == NULL)
        return NULL;

    if (!this->parser->ReadByteAlignedData(data, size))
    {
        free(data);
        return NULL;
    }

    *outChecksum = this->entries[entryIdx].checksum;
    *outSize = size;
    return data;
}

Pbg3Archive::~Pbg3Archive()
{
    this->Release();
}

i32 Pbg3Archive::Load(const char *path)
{
    if (!this->Release())
    {
        return false;
    }

    this->parser = new Pbg3Parser();
    if (this->parser == NULL)
    {
        return false;
    }

    if (!this->parser->OpenArchive(path))
    {
        if (this->parser != NULL)
        {
            delete this->parser;
            this->parser = NULL;
        }
        return false;
    }

    const char *basename = path;
    for (const char *cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '/' || *cursor == '\\')
        {
            basename = cursor + 1;
        }
    }
    const char *extension = std::strrchr(basename, '.');
    const size_t stemLength = extension != NULL ? (size_t)(extension - basename) : std::strlen(basename);
    std::snprintf(this->sidecarDirectory, sizeof(this->sidecarDirectory), "data/%.*s", (int)stemLength, basename);

    const i32 parsed = this->ParseHeader();
#ifdef __PSP__
    // PSP reads entry payloads from extracted sidecars, so the DAT stream is
    // no longer needed after its small filename table has been parsed. Close
    // it before title/game transitions to reduce open handles and I/O state.
    if (parsed && this->parser != NULL)
    {
        delete this->parser;
        this->parser = NULL;
    }
#endif
    return parsed;
}

u8 *Pbg3Archive::ReadExtractedEntry(u32 entryIdx)
{
    if (entryIdx >= this->numOfEntries || this->sidecarDirectory[0] == '\0')
    {
        return NULL;
    }

    char path[384];
    std::snprintf(path, sizeof(path), "%s/%s", this->sidecarDirectory, this->entries[entryIdx].filename);
    FILE *file = FileSystem::FopenUTF8(path, "rb");
    if (file == NULL)
    {
        return NULL;
    }

    std::fseek(file, 0, SEEK_END);
    const long fileSize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    if (fileSize < 0 || (u32)fileSize != this->entries[entryIdx].uncompressedSize)
    {
        std::fclose(file);
        return NULL;
    }

    u8 *data = (u8 *)std::malloc(fileSize == 0 ? 1 : (size_t)fileSize);
    if (data == NULL || (fileSize > 0 && std::fread(data, 1, (size_t)fileSize, file) != (size_t)fileSize))
    {
        std::free(data);
        std::fclose(file);
        return NULL;
    }

    std::fclose(file);
#ifdef __PSP__
    std::printf("PBG3: using extracted asset %s\n", path);
#endif
    return data;
}

#define LZSS_DICTSIZE 0x2000
#define LZSS_DICTSIZE_MASK 0x1fff
#define LZSS_MIN_MATCH 3

namespace
{
class Pbg3BitReader
{
  public:
    Pbg3BitReader(const u8 *data, u32 size) : data(data), size(size)
    {
    }

    bool ReadBit(u32 &bit)
    {
        if (this->mask == 0x80)
        {
            if (this->position >= this->size)
            {
                return false;
            }
            this->current = this->data[this->position++];
            this->checksum += this->current;
        }

        bit = (this->current & this->mask) != 0;
        this->mask >>= 1;
        if (this->mask == 0)
        {
            this->mask = 0x80;
        }
        return true;
    }

    bool ReadBits(u32 count, u32 &value)
    {
        value = 0;
        for (u32 bitIndex = 0; bitIndex < count; ++bitIndex)
        {
            u32 bit;
            if (!this->ReadBit(bit))
            {
                return false;
            }
            value = (value << 1) | bit;
        }
        return true;
    }

    u32 GetChecksum() const
    {
        return this->checksum;
    }

  private:
    const u8 *data;
    u32 size;
    u32 position = 0;
    u32 current = 0;
    u32 checksum = 0;
    u8 mask = 0x80;
};

#ifdef __PSP__
void LogDecodeFailure(const char *filename, const char *reason, u32 expectedSize, u32 actualSize, u32 expectedChecksum,
                      u32 actualChecksum)
{
    std::printf("PBG3: %s failed (%s), size %u/%u, checksum %08x/%08x\n", filename, reason,
                (unsigned int)actualSize, (unsigned int)expectedSize, (unsigned int)actualChecksum,
                (unsigned int)expectedChecksum);
}
#else
void LogDecodeFailure(const char *, const char *, u32, u32, u32, u32)
{
}
#endif
} // namespace

u8 *Pbg3Archive::ReadDecompressEntry(u32 entryIdx, const char *filename)
{
    if (entryIdx >= this->numOfEntries || this->parser == NULL)
        return NULL;

    const u32 outputSize = this->GetEntrySize(entryIdx);
    u8 *out = (u8 *)malloc(outputSize == 0 ? 1 : outputSize);
    if (out == NULL)
    {
        LogDecodeFailure(filename, "output allocation", outputSize, 0, this->entries[entryIdx].checksum, 0);
        return NULL;
    }

    u32 compressedSize = 0;
    u32 expectedCsum = this->entries[entryIdx].checksum;
    u8 *rawData = this->ReadEntryRaw(&compressedSize, &expectedCsum, entryIdx);

    if (rawData == NULL)
    {
        LogDecodeFailure(filename, "archive read", outputSize, 0, expectedCsum, 0);
        free(out);
        return NULL;
    }

    u8 *dict = new (std::nothrow) u8[LZSS_DICTSIZE]();
    if (dict == NULL)
    {
        LogDecodeFailure(filename, "dictionary allocation", outputSize, 0, expectedCsum, 0);
        free(rawData);
        free(out);
        return NULL;
    }

    Pbg3BitReader reader(rawData, compressedSize);
    u32 dictHead = 1;
    u32 outputPosition = 0;
    bool success = true;

    for (;;)
    {
        u32 opcode;
        if (!reader.ReadBit(opcode))
        {
            success = false;
            break;
        }

        if (opcode != 0)
        {
            u32 literal;
            if (!reader.ReadBits(8, literal) || outputPosition >= outputSize)
            {
                success = false;
                break;
            }
            out[outputPosition++] = (u8)literal;
            dict[dictHead] = (u8)literal;
            dictHead = (dictHead + 1) & LZSS_DICTSIZE_MASK;
        }
        else
        {
            u32 matchOffset;
            if (!reader.ReadBits(13, matchOffset))
            {
                success = false;
                break;
            }
            if (matchOffset == 0)
            {
                break;
            }

            u32 lengthCode;
            if (!reader.ReadBits(4, lengthCode))
            {
                success = false;
                break;
            }

            const u32 length = lengthCode + LZSS_MIN_MATCH;
            if (length > outputSize - outputPosition)
            {
                success = false;
                break;
            }
            for (u32 i = 0; i < length; ++i)
            {
                const u8 value = dict[(matchOffset + i) & LZSS_DICTSIZE_MASK];
                out[outputPosition++] = value;
                dict[dictHead] = value;
                dictHead = (dictHead + 1) & LZSS_DICTSIZE_MASK;
            }
        }
    }

    const u32 actualChecksum = reader.GetChecksum();
    delete[] dict;
    free(rawData);

    if (!success || outputPosition != outputSize || expectedCsum != actualChecksum)
    {
        const char *reason = !success ? "invalid stream" : outputPosition != outputSize ? "output size" : "checksum";
        LogDecodeFailure(filename, reason, outputSize, outputPosition, expectedCsum, actualChecksum);
        free(out);
        out = NULL;
        return NULL;
    }

    return out;
}
