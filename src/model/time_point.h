#ifndef ALGORITHMIC_TRADING_MODEL_TIMEPOINT_H
#define ALGORITHMIC_TRADING_MODEL_TIMEPOINT_H

#include <chrono>
#include <string>

class TimePoint {
 public:
  TimePoint(time_t time);
  TimePoint(std::string& date);

  std::string ToString() const;
  time_t ToTime_t() const;
  time_t AddDays(int days) const;
  double ToDouble() const;
  bool isValid() const;
  std::chrono::time_point<std::chrono::high_resolution_clock>& Value();

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> data_;
  bool isValid_;
};

#endif  // ALGORITHMIC_TRADING_MODEL_TIMEPOINT_H
