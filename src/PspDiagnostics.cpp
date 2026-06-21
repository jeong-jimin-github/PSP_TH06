#include "PspDiagnostics.hpp"

#ifdef __PSP__
#include <pspsysmem.h>
#include <malloc.h>
#include <cstdio>

namespace
{
bool g_IsTracingStageLoad;

void WriteTrace(const char *marker, const char *detail, const char *mode)
{
    FILE *file = std::fopen("./loadtrace.txt", mode);
    if (file == nullptr)
    {
        return;
    }

    std::fprintf(file, "%s", marker);
    if (detail != nullptr)
    {
        std::fprintf(file, " %s", detail);
    }
    const struct mallinfo heap = mallinfo();
    std::fprintf(file, " heap_used=%lu heap_free=%lu system_free=%lu system_max=%lu\n",
                 static_cast<unsigned long>(heap.uordblks), static_cast<unsigned long>(heap.fordblks),
                 static_cast<unsigned long>(sceKernelTotalFreeMemSize()),
                 static_cast<unsigned long>(sceKernelMaxFreeMemSize()));
    std::fflush(file);
    std::fclose(file);
}

} // namespace
#endif

namespace PspDiagnostics
{
void ResetLoadTrace()
{
#ifdef __PSP__
    g_IsTracingStageLoad = false;
    WriteTrace("boot", nullptr, "w");
#endif
}

void BeginStageLoad()
{
#ifdef __PSP__
    g_IsTracingStageLoad = true;
    WriteTrace("stage_load_begin", nullptr, "a");
#endif
}

void TraceStageLoad(const char *marker, const char *detail)
{
#ifdef __PSP__
    if (g_IsTracingStageLoad)
    {
        WriteTrace(marker, detail, "a");
    }
#else
    (void)marker;
    (void)detail;
#endif
}

void EndStageLoad()
{
#ifdef __PSP__
    WriteTrace("stage_load_complete", nullptr, "a");
    g_IsTracingStageLoad = false;
#endif
}
} // namespace PspDiagnostics
