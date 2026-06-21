#pragma once

namespace PspDiagnostics
{
void ResetLoadTrace();
void BeginStageLoad();
void TraceStageLoad(const char *marker, const char *detail = nullptr);
void EndStageLoad();
} // namespace PspDiagnostics
