#include <benchmark/benchmark.h>
#include <sys/resource.h>

#include <memory>

#include "mytinyhttpd/base/async_logging.h"
#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/utils/constants.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

static void BM_AsyncLoggingAppend(benchmark::State& state) {
  auto roll_size = state.range(0), test_short_message = state.range(1);
  std::string message =
      test_short_message ? "short message" : std::string(3000, 'x');
  AsyncLogging async_logging(test_short_message ? "async_logging_short_bench"
                                                : "async_logging_long_bench",
                             roll_size);
  Logger::SetOutput(
      [&](const char* msg, int len) { async_logging.Append(msg, len); });
  async_logging.Start();
  for (auto _ : state) {
    LOG_INFO << message;
  }
}
BENCHMARK(BM_AsyncLoggingAppend)
    ->RangeMultiplier(10)
    ->Ranges({{10 * kBytesPerMegabyte, 1000 * kBytesPerMegabyte}, {0, 1}});

}  // namespace mytinyhttpd

int main(int argc, char** argv) {
  {
    // set max virtual memory to 2GB.
    rlimit rl = {2 * kBytesPerGigabyte, 2 * kBytesPerGigabyte};
    setrlimit(RLIMIT_AS, &rl);
  }
  {
    using namespace mytinyhttpd;
    TimeZone beijing(8 * 3600, "CST");
    Logger::SetTimeZone(beijing);
  }
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return 0;
}