#include <benchmark/benchmark.h>

#include "sbovector.hpp"
#include "sbovector_old.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <vector>

// Some benchmarks to help with improving SBOVector,
// Test improvements against sbovector_old.hpp to have a point of comparison
// Bear in mind, many compilers heavily optimize std::vector
// and benchmark::DoNotOptimize has its limitations as to what it forces preservation of
// in optimized builds so these benchmarks should be treated with heavy, heavy skepticism.

template<typename ContainerType>
auto CreateValueForContainer() {
  std::remove_reference_t<decltype(std::declval<ContainerType>().at(0))> out(42);
  return out;
}

template<typename ContainerType>
void BM_DefaultConstruct(benchmark::State& state) {
  for (auto _ : state) {
    ContainerType container;
    benchmark::DoNotOptimize(container);
  }
}

template<typename ContainerType>
void BM_CountConsturct(benchmark::State& state) {
  for (auto _ : state) {
    const auto count = static_cast<size_t>(state.range(0));
    ContainerType container(count);
  }
}

template<typename ContainerType>
void BM_CountValueConstructor(benchmark::State& state) {
  for (auto _ : state) {
    const auto count = static_cast<size_t>(state.range(0));
    const auto value = CreateValueForContainer<ContainerType>();
    ContainerType container(count, value);
  }
}

template<typename ContainerType>
void BM_RangeConstructor(benchmark::State& state) {
  using DataType = std::remove_reference_t<decltype(std::declval<ContainerType>().at(0))>;
  std::vector<DataType> vec;
  vec.reserve(static_cast<size_t>(state.range(0)));
  for (auto i = 0u; i < state.range(0); ++i) {
    vec.push_back(CreateValueForContainer<ContainerType>());
  }

  for (auto _ : state) {
    ContainerType container(vec.begin(), vec.end());
  }
}

template<typename ContainerType>
void BM_CopyConstructor(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  const ContainerType original(count, value);
  for (auto _ : state) {
    ContainerType copy(original);
  }
}

template<typename ContainerType>
void BM_MoveConstructor(benchmark::State& state) {
  std::aligned_storage_t<sizeof(ContainerType), alignof(ContainerType)>
      container_space;
  auto poriginal = reinterpret_cast<ContainerType*>(&container_space);
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  new(poriginal) ContainerType(count, value);
  for (auto _ : state) {
    ContainerType temp(std::move(*poriginal));
    std::destroy_at(poriginal);
    new (poriginal) ContainerType(std::move(temp));
  }
  std::destroy_at(poriginal);
}

template<typename ContainerType>
void BM_AssignOperatorCopy(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  ContainerType a(count, value), b;
  for (auto _ : state) {
    b = a;
    a = b;
  }
}

template <typename ContainerType>
void BM_AssignOperatorMove(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  ContainerType a(count, value), b;
  for (auto _ : state) {
    b = std::move(a);
    a = std::move(b);
  }
}
template <typename ContainerType>
void BM_AssignCountValue(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  ContainerType a(count, value);
  for (auto _ : state) {
    a.assign(count, value);
  }
}

template <typename ContainerType>
void BM_AssignRange(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  std::vector vec(count, value);
  ContainerType c;
  for (auto _ : state) {
    c.assign(vec.begin(), vec.end());
  }
}

template<typename ContainerType>
void BM_Iterate(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  ContainerType a(count, value);
  for (auto _ : state) {
    auto total = std::accumulate(a.begin(), a.end(), value);
    benchmark::DoNotOptimize(total);
  }
}

template<typename ContainerType>
void BM_SequentialIteration(benchmark::State& state) {
  std::array<ContainerType, 1000> containers;
  const auto value = CreateValueForContainer<ContainerType>();
  for (size_t i = 0u; i < containers.size(); ++i) {
    containers[i].assign((i % 997) ? (i % 2 ? 8 : 16) : 500, value);
  }
  for (auto _ : state) {
    for (auto& container : containers) {
      auto total = std::accumulate(container.begin(), container.end(), value);
      benchmark::DoNotOptimize(total);
    }
  }
}

template<typename ContainerType>
void BM_InsertSingle(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  for (auto _ : state) {
    state.PauseTiming();
    { 
      ContainerType c;
      state.ResumeTiming();
      for (auto i = 0u; i < count; ++i) {
        c.insert(c.begin(), value);
      }
      state.PauseTiming();
    }
    state.ResumeTiming();
  
  }
}

template <typename ContainerType>
void BM_InsertCount(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  for (auto _ : state) {
    state.PauseTiming();
    {
      ContainerType c;
      state.ResumeTiming();
      for (auto i = 0u; i < count; ++i) {
        c.insert(c.begin(), 5, value);
      }
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}

template <typename ContainerType>
void BM_PushBack(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  for (auto _ : state) {
    state.PauseTiming();
    {
      ContainerType c;
      state.ResumeTiming();
      for (auto i = 0u; i < count; ++i) {
        c.push_back(value);
      }
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}

template <typename ContainerType>
void BM_EraseSingle(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  for (auto _ : state) {
    state.PauseTiming();
    {
      ContainerType c(count, value);
      state.ResumeTiming();
      for (auto i = 0u; i < count; ++i) {
        c.erase(c.begin());
      }
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}

template <typename ContainerType>
void BM_EraseCount(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  for (auto _ : state) {
    state.PauseTiming();
    {
      ContainerType c(count, value);
      state.ResumeTiming();
      for (auto i = 0u; (i + 6) < count; i += 6) {
        c.erase(c.begin(), c.begin() + 6);
      }
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}

template <typename ContainerType>
void BM_PopBack(benchmark::State& state) {
  const auto count = static_cast<size_t>(state.range(0));
  const auto value = CreateValueForContainer<ContainerType>();
  for (auto _ : state) {
    state.PauseTiming();
    {
      ContainerType c(count, value);
      state.ResumeTiming();
      for (auto i = 0u; i < count; ++i) {
        c.pop_back();
      }
      state.PauseTiming();
    }
    state.ResumeTiming();
  }
}

template<typename ContainerType>
void BM_Resize(benchmark::State& state) {
  ContainerType c;
  const auto count = static_cast<size_t>(state.range(0)) + 10;
  for (auto _ : state) {
    for (auto i = 10u; i < count; ++i) {
      c.resize(i % 2 ? i : i / 2);
    }
  }
}

template<typename ContainerType>
void BM_Swap(benchmark::State& state) {
  const auto count_a = static_cast<size_t>(state.range(0));
  const auto count_b = static_cast<size_t>(state.range(1));
  ContainerType a(count_a);
  ContainerType b(count_b);
  for (auto _ : state) {
    a.swap(b);
  }
}

#define BM_TEMPLATE(NAME, TYPE) BENCHMARK_TEMPLATE(NAME, TYPE)

#define BM_TEMPLATER(NAME, TYPE, LOW, HIGH) \
  BENCHMARK_TEMPLATE(NAME, TYPE)->Range(LOW, HIGH)

#define BM_TEMPLATERM(NAME, TYPE, MUL, LOW, HIGH) \
  BENCHMARK_TEMPLATE(NAME,TYPE)->RangeMultiplier(MUL)->Range(LOW, HIGH)

#define CONSTRUCTOR_BENCHMARKS(TYPE) \
  BM_TEMPLATE(BM_DefaultConstruct, TYPE); \
  BM_TEMPLATERM(BM_CountConsturct, TYPE, 2, 8, 64); \
  BM_TEMPLATERM(BM_CountValueConstructor, TYPE, 2, 8, 64); \
  BM_TEMPLATERM(BM_RangeConstructor, TYPE, 2, 8, 64);\
  BM_TEMPLATERM(BM_CopyConstructor, TYPE, 2, 8, 64);\
  BM_TEMPLATERM(BM_MoveConstructor, TYPE, 2, 8, 64)

#define ASSIGNMENT_BENCHMARKS(TYPE) \
  BM_TEMPLATERM(BM_AssignOperatorCopy, TYPE, 2, 8, 64);\
  BM_TEMPLATERM(BM_AssignOperatorMove, TYPE, 2, 8, 64);\
  BM_TEMPLATERM(BM_AssignCountValue, TYPE, 2, 8, 64); \
  BM_TEMPLATERM(BM_AssignRange, TYPE, 2, 8, 64)

#define ITERATION_BENCHMARKS(TYPE) \
  BM_TEMPLATERM(BM_Iterate, TYPE, 2, 8, 64); \
  BM_TEMPLATE(BM_SequentialIteration, TYPE)

#define MODIFY_BENCHMARKS(TYPE) \
  BM_TEMPLATERM(BM_InsertSingle, TYPE, 2, 8, 32);\
  BM_TEMPLATERM(BM_InsertCount, TYPE, 2, 8, 32);\
  BM_TEMPLATERM(BM_PushBack, TYPE, 2, 8, 32);\
  BM_TEMPLATERM(BM_EraseSingle, TYPE, 2, 8, 32); \
  BM_TEMPLATERM(BM_EraseCount, TYPE, 2, 8, 32); \
  BM_TEMPLATERM(BM_PopBack, TYPE, 2, 8, 32);\
  BM_TEMPLATERM(BM_Resize, TYPE, 2, 8, 32)

#define SWAP_BENCHMARKS(TYPE)\
  BM_TEMPLATE(BM_Swap, TYPE)->Args({8, 16})->Args({16, 32})->Args({32, 32})

#define TEMPLATE_ALL_BENCHMARKS(TYPE) \
  CONSTRUCTOR_BENCHMARKS(TYPE); \
  ASSIGNMENT_BENCHMARKS(TYPE); \
  ITERATION_BENCHMARKS(TYPE); \
  MODIFY_BENCHMARKS(TYPE); \
  SWAP_BENCHMARKS(TYPE)

typedef SBOVector<int, 16> SBOVectorInt16;
typedef old::SBOVector<int, 16> OldSBOVectorInt16;

//SWAP_BENCHMARKS(std::vector<int>);
TEMPLATE_ALL_BENCHMARKS(SBOVectorInt16);
TEMPLATE_ALL_BENCHMARKS(OldSBOVectorInt16);

BENCHMARK_MAIN();