#pragma once

namespace PspDiagnostics
{
void ResetLoadTrace();
void BeginStageLoad();
void TraceStageLoad(const char *marker, const char *detail = nullptr);
void TraceRuntime(const char *marker, int priority = -1);
void EndStageLoad();
} // namespace PspDiagnostics
