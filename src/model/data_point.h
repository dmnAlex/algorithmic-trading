#ifndef ALGORITHMIC_TRADING_MODEL_DATAPOINT_H
#define ALGORITHMIC_TRADING_MODEL_DATAPOINT_H

#include "time_point.h"

struct DataPoint {
  DataPoint(TimePoint date, double price) : date(date), price(price) {}

  TimePoint date;
  double price;
};

#endif  // ALGORITHMIC_TRADING_MODEL_DATAPOINT_H
