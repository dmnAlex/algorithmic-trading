#include "time_point.h"

#include <iomanip>

TimePoint::TimePoint(std::string& date) {
  std::tm tm{};
  std::istringstream ss(date);
  ss >> std::get_time(&tm, "%Y-%m-%d");

  if (!ss.fail()) {
    std::time_t time = std::mktime(&tm);
    data_ = std::chrono::high_resolution_clock::from_time_t(time);
    isValid_ = true;
  } else {
    isValid_ = false;
  }
}

TimePoint::TimePoint(time_t time)
    : data_(std::chrono::high_resolution_clock::from_time_t(time)),
      isValid_(true) {}

std::string TimePoint::ToString() const {
  if (!isValid_) {
    return "(null)";
  }

  auto time = std::chrono::high_resolution_clock::to_time_t(data_);
  std::tm tm = *std::localtime(&time);
  std::ostringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d");

  return ss.str();
}

double TimePoint::ToDouble() const {
  return std::chrono::duration<double>(data_.time_since_epoch()).count();
}

time_t TimePoint::ToTime_t() const {
  return std::chrono::high_resolution_clock::to_time_t(data_);
}

time_t TimePoint::AddDays(int days) const {
  return std::chrono::high_resolution_clock::to_time_t(
      data_ + std::chrono::hours(days * 24));
}

bool TimePoint::isValid() const { return isValid_; }

std::chrono::time_point<std::chrono::high_resolution_clock>&
TimePoint::Value() {
  return data_;
}
