#include <benchmark/benchmark.h>

#include "sbovector.hpp"

#include <array>
#include <vector>

template<typename ContainerType, auto ElementProvider>
void BM_LazyConstructLarge(benchmark::State& state) {
  for (auto _ : state) {
    ContainerType container{};
    for (auto i = 0u; i < 1000; ++i) {
      container.push_back(ElementProvider());
    }
  }
}

template<typename ContainerType, auto ElementProvider, auto ElementConsumer>
void BM_MotivatingExample(benchmark::State& state) {
  // Motivation for a SBOVector is an iteration over a collection of containers almost all (> 99%) of which are tiny
  // but must be able to handle a large container.
  // Example usage: Consider drawing a collection of objects where the objects are represented by a client defined
  // bytecode that executes the custom drawing of the object, for almost all objects the bytecode is relatively tiny
  // ... for(const auto& object : objects) execute(object.data());
  // should benifit from a SBO structure by reducing cache misses
  std::array<ContainerType, 1000> containers{};
  for (auto& container : containers) {
    for (auto i = 0u; i < 10; ++i) {
      container.push_back(ElementProvider());
    }
  }
  for (auto i = 0u; i < 500; ++i) {
    containers[7].push_back(ElementProvider());
  }

  for (auto _ : state) {
    for (const auto& container : containers) {
      for (const auto& element : container) {
        ElementConsumer(element);
      }
    }
  }
}

int int_provider() {
  return 0;
}

void int_consumer(const int& i) {
  static int total{0};
  total += i;
}

BENCHMARK_TEMPLATE(BM_LazyConstructLarge, std::vector<int>, int_provider);
BENCHMARK_TEMPLATE(BM_MotivatingExample,
                    std::vector<int>,
                    int_provider,
                    int_consumer);

BENCHMARK_TEMPLATE(BM_LazyConstructLarge, SBOVector<int, 16>, int_provider);
BENCHMARK_TEMPLATE(BM_MotivatingExample,
                   SBOVector<int, 16>,
                   int_provider,
                   int_consumer);

BENCHMARK_MAIN();