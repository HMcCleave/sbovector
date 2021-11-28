#include "unittest_common.hpp"

OperationCounter::OperationTotals OperationCounter::TOTALS {};
std::mutex DataTypeOperationTrackingSBOVector::SharedDataMutex{};