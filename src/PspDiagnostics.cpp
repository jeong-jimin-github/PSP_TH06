#include "PspDiagnostics.hpp"

#ifdef __PSP__
#include <pspsysmem.h>
#include <malloc.h>
#include <cstdio>

namespace
{
bool g_IsTracingStageLoad;
bool g_IsTracingRuntime;
unsigned int g_RuntimeTraceCount;
constexpr unsigned int RUNTIME_TRACE_LIMIT = 96;

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
    g_IsTracingRuntime = false;
    g_RuntimeTraceCount = 0;
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

void TraceRuntime(const char *marker, int priority)
{
#ifdef __PSP__
    if (!g_IsTracingRuntime || g_RuntimeTraceCount >= RUNTIME_TRACE_LIMIT)
    {
        return;
    }

    char detail[24];
    const char *detailPtr = nullptr;
    if (priority >= 0)
    {
        std::snprintf(detail, sizeof(detail), "priority=%d", priority);
        detailPtr = detail;
    }
    WriteTrace(marker, detailPtr, "a");
    g_RuntimeTraceCount++;
#else
    (void)marker;
    (void)priority;
#endif
}

void EndStageLoad()
{
#ifdef __PSP__
    WriteTrace("stage_load_complete", nullptr, "a");
    g_IsTracingRuntime = true;
    g_RuntimeTraceCount = 0;
#endif
}
} // namespace PspDiagnostics
