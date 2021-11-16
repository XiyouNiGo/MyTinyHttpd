#include "mytinyhttpd/base/process_info.h"

#include <benchmark/benchmark.h>

namespace mytinyhttpd {

namespace process_info {

static void BM_GetPid(benchmark::State& state) {
  for (auto _ : state) pid();
}
BENCHMARK(BM_GetPid);

static void BM_GetPidString(benchmark::State& state) {
  for (auto _ : state) pid_string();
}
BENCHMARK(BM_GetPidString);

static void BM_GetUid(benchmark::State& state) {
  for (auto _ : state) uid();
}
BENCHMARK(BM_GetUid);

static void BM_GetUsername(benchmark::State& state) {
  for (auto _ : state) username();
}
BENCHMARK(BM_GetUsername);

static void BM_GetEuid(benchmark::State& state) {
  for (auto _ : state) euid();
}
BENCHMARK(BM_GetEuid);

static void BM_GetStartTime(benchmark::State& state) {
  for (auto _ : state) start_time();
}
BENCHMARK(BM_GetStartTime);

static void BM_GetClockTicksPerSecond(benchmark::State& state) {
  for (auto _ : state) clock_ticks_per_second();
}
BENCHMARK(BM_GetClockTicksPerSecond);

static void BM_GetPageSize(benchmark::State& state) {
  for (auto _ : state) page_size();
}
BENCHMARK(BM_GetPageSize);

static void BM_GetIsDebugBuild(benchmark::State& state) {
  for (auto _ : state) IsDebugBuild();
}
BENCHMARK(BM_GetIsDebugBuild);

static void BM_GetHostname(benchmark::State& state) {
  for (auto _ : state) hostname();
}
BENCHMARK(BM_GetHostname);

static void BM_GetProcname(benchmark::State& state) {
  for (auto _ : state) procname();
}
BENCHMARK(BM_GetProcname);

static void BM_GetProcStatus(benchmark::State& state) {
  for (auto _ : state) proc_status();
}
BENCHMARK(BM_GetProcStatus);

static void BM_GetProcStat(benchmark::State& state) {
  for (auto _ : state) proc_stat();
}
BENCHMARK(BM_GetProcStat);

static void BM_GetThreadStat(benchmark::State& state) {
  for (auto _ : state) thread_stat();
}
BENCHMARK(BM_GetThreadStat);

static void BM_GetExePath(benchmark::State& state) {
  for (auto _ : state) exe_path();
}
BENCHMARK(BM_GetExePath);

static void BM_GetOpenedFiles(benchmark::State& state) {
  for (auto _ : state) opened_files();
}
BENCHMARK(BM_GetOpenedFiles);

static void BM_GetMaxOpenFiles(benchmark::State& state) {
  for (auto _ : state) max_open_files();
}
BENCHMARK(BM_GetMaxOpenFiles);

static void BM_GetCpuTimed(benchmark::State& state) {
  for (auto _ : state) cpu_time();
}
BENCHMARK(BM_GetCpuTimed);

static void BM_GetNumThreads(benchmark::State& state) {
  for (auto _ : state) num_threads();
}
BENCHMARK(BM_GetNumThreads);

}  // namespace process_info

}  // namespace mytinyhttpd

BENCHMARK_MAIN();
