#include <benchmark/benchmark.h>

void BM_Sample(benchmark::State& state) {
  int i{0};
  for (auto _ : state) {
    ++i;
  }
}

BENCHMARK(BM_Sample);

BENCHMARK_MAIN();