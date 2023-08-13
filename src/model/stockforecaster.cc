#include "stockforecaster.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

bool StockForecaster::LoadData(const std::string& file_path) {
  bool success = true;
  std::vector<DataPoint> data;

  std::ifstream ifs(file_path);
  if (!ifs.is_open()) {
    error_message_ = "Unable to open file: " + file_path;
    success = false;
  } else {
    std::string line;
    std::getline(ifs, line);

    while (std::getline(ifs, line)) {
      std::istringstream sstream(line);

      std::string date;
      std::getline(sstream, date, ',');
      TimePoint time_point(date);

      double price;
      sstream >> price;

      if (sstream.fail() || !time_point.isValid()) {
        error_message_ = "File has invalid data: " + line;
        success = false;
        break;
      }

      if (!data.empty() && time_point.Value() <= data.back().date.Value()) {
        error_message_ = "File data must be sorted in ascending order: " + line;
        success = false;
        break;
      }

      data.emplace_back(time_point, price);
    }

    ifs.close();
  }

  if (success) {
    data_ = data;
  }

  return success;
}

bool StockForecaster::InterpolatePriceByCubicSplineMethod(time_t date) {
  if (data_.empty()) {
    error_message_ = "First you need to load the data";
    return false;
  }

  std::array<std::vector<double>, 4> coeffs = DefineInterpolationCoefficients();
  int pivot_date_idx = DefinePivotDateIndex(date);

  forecast_price_ = InterpolatePrice(date, coeffs, pivot_date_idx);

  return true;
}

bool StockForecaster::InterpolatePricesByCubicSplineMethod(int dates_count) {
  if (data_.empty()) {
    error_message_ = "First you need to load the data";
    return false;
  }

  time_t period = data_.back().date.ToTime_t() - data_.front().date.ToTime_t();
  std::vector<time_t> dates = DefineDates(dates_count, period);

  std::array<std::vector<double>, 4> coeffs = DefineInterpolationCoefficients();

  forecast_ = std::vector<DataPoint>();
  forecast_.reserve(dates.size());
  for (size_t i = 0; i < dates.size(); ++i) {
    int pivot_date_idx = DefinePivotDateIndex(dates[i]);
    double price = InterpolatePrice(dates[i], coeffs, pivot_date_idx);
    forecast_.emplace_back(dates[i], price);
  }

  return true;
}

bool StockForecaster::ApproximatePriceByLeastSquaresMethod(time_t date,
                                                           int degree) {
  if (data_.empty()) {
    error_message_ = "First you need to load the data";
    return false;
  }

  std::vector<double> coeffs = DefineApproximationCoefficients(degree);
  forecast_price_ = ApproximatePrice(date, coeffs);

  return true;
}

bool StockForecaster::ApproximatePricesByLeastSquaresMethod(int dates_count,
                                                            int future_days,
                                                            int degree) {
  if (data_.empty()) {
    error_message_ = "First you need to load the data";
    return false;
  }

  time_t period =
      data_.back().date.AddDays(future_days) - data_.front().date.ToTime_t();
  std::vector<time_t> dates = DefineDates(dates_count, period);

  std::vector<double> coeffs = DefineApproximationCoefficients(degree);

  forecast_ = std::vector<DataPoint>();
  forecast_.reserve(dates.size());
  for (size_t i = 0; i < dates.size(); ++i) {
    double price = ApproximatePrice(dates[i], coeffs);
    forecast_.emplace_back(dates[i], price);
  }

  return true;
}

time_t StockForecaster::GetMaxDate() const {
  return data_.back().date.ToTime_t();
}

time_t StockForecaster::GetMinDate() const {
  return data_.front().date.ToTime_t();
}

const std::string& StockForecaster::GetError() const { return error_message_; }

double StockForecaster::GetForecastPrice() const { return forecast_price_; }

const std::vector<DataPoint>& StockForecaster::GetForecast() const {
  return forecast_;
}

const std::vector<DataPoint>& StockForecaster::GetData() const { return data_; }

// COMMON METHODS

std::vector<time_t> StockForecaster::DefineDates(int dates_count,
                                                 time_t period) {
  time_t interval_length = period / (dates_count - 1);
  std::vector<time_t> dates;

  time_t date_i = data_.front().date.ToTime_t();  // X0
  for (int i = 0; i < dates_count; ++i) {
    dates.push_back(date_i);
    date_i += interval_length;
  }

  return dates;
}

std::vector<double> StockForecaster::SolveSle(Matrix& sle) {
  int last_row = sle.size() - 1;
  int last_col = sle.front().size() - 1;

  // forward elimination
  int pivot_row = 0, pivot_col = 0;
  while (pivot_row < last_row) {
    for (int i = pivot_row + 1; i <= last_row; ++i) {
      double multiplier = sle[i][pivot_col] / sle[pivot_row][pivot_col];
      for (int j = pivot_col; j <= last_col; ++j) {
        sle[i][j] -= sle[pivot_row][j] * multiplier;
      }
    }

    ++pivot_row;
    ++pivot_col;
  }

  // back substitution
  std::vector<double> solution = std::vector<double>(sle.size());
  for (int i = last_row; i >= 0; --i) {
    int j = last_col - 1;
    for (; j > i; --j) {
      sle[i][last_col] -= sle[i][j] * solution[j];
    }

    solution[i] = sle[i][last_col] / sle[i][j];
  }

  return solution;
}

// INTERPOLATION METHODS

std::array<std::vector<double>, 4>
StockForecaster::DefineInterpolationCoefficients() {
  std::vector<time_t> dx(data_.size());  // date intervals
  std::vector<double> dy(data_.size());  // price intervals
  for (int i = 1; i < data_.size(); ++i) {
    dx[i] = data_[i].date.ToTime_t() - data_[i - 1].date.ToTime_t();
    dy[i] = data_[i].price - data_[i - 1].price;
  }

  // make SLE
  Matrix sle(data_.size(), std::vector<double>(data_.size() + 1));
  sle[0][0] = 1;
  sle[data_.size() - 1][data_.size() - 1] = 1;
  for (int i = 1; i < sle.size() - 1; ++i) {
    sle[i][i - 1] = dx[i];
    sle[i][i] = 2 * (dx[i] + dx[i + 1]);
    sle[i][i + 1] = dx[i + 1];
    sle[i].back() = 3.0 * (dy[i + 1] / dx[i + 1] - dy[i] / dx[i]);
  }

  // calculate coefficients
  std::array<std::vector<double>, 4> coeffs;
  coeffs[C] = SolveSle(sle);
  coeffs[A] = std::vector<double>(data_.size());
  coeffs[B] = std::vector<double>(data_.size());
  coeffs[D] = std::vector<double>(data_.size());

  coeffs[A].front() = data_.front().price;
  for (int i = 1; i < data_.size(); ++i) {
    coeffs[A][i] = data_[i].price;
    coeffs[B][i] =
        dy[i] / dx[i] + (2.0 * coeffs[C][i] + coeffs[C][i - 1]) / 3.0 * dx[i];
    coeffs[D][i] = (coeffs[C][i] - coeffs[C][i - 1]) / (3.0 * dx[i]);
  }

  return coeffs;
}

int StockForecaster::DefinePivotDateIndex(time_t date) {
  size_t pivot_date_idx = 0;
  while (date > data_[pivot_date_idx].date.ToTime_t()) {
    ++pivot_date_idx;
  }

  return pivot_date_idx;
}

double StockForecaster::InterpolatePrice(
    time_t date, const std::array<std::vector<double>, 4>& coeffs,
    int pivot_date_idx) {
  time_t delta = date - data_[pivot_date_idx].date.ToTime_t();
  return coeffs[A][pivot_date_idx] + coeffs[B][pivot_date_idx] * delta +
         coeffs[C][pivot_date_idx] * std::pow(delta, 2) +
         coeffs[D][pivot_date_idx] * std::pow(delta, 3);
}

// APPROXIMATION METHODS

std::vector<double> StockForecaster::DefineApproximationCoefficients(
    int degree) {
  // make SLE
  Matrix sle(degree + 1, std::vector<double>(degree + 2));
  int last_col = sle.front().size() - 1;

  // fill first line of X
  sle[0][0] = data_.size();
  for (int i = 1; i < last_col; ++i) {
    double sum = 0.0;
    for (int j = 0; j < data_.size(); ++j) {
      sum += std::pow(data_[j].date.ToTime_t(), i);
    }

    sle[0][i] = sum;
  }

  // fill penault column (X with largest degree in each row)
  for (int i = 1; i <= degree; ++i) {
    double sum = 0.0;
    for (int j = 0; j < data_.size(); ++j) {
      sum += std::pow(data_[j].date.ToTime_t(), degree + i);
    }

    sle[i][last_col - 1] = sum;
  }

  // fill last column (Y)
  for (int i = 0; i <= degree; ++i) {
    double sum = 0.0;
    for (int j = 0; j < data_.size(); ++j) {
      sum += data_[j].price * std::pow(data_[j].date.ToTime_t(), i);
    }

    sle[i][last_col] = sum;
  }

  // fill other cells
  for (int i = 1; i <= degree; ++i) {
    for (int j = 0; j < last_col - 1; ++j) {
      sle[i][j] = sle[i - 1][j + 1];
    }
  }

  return SolveSle(sle);
}

double StockForecaster::ApproximatePrice(time_t date,
                                         std::vector<double> coeffs) {
  double price = 0.0;
  for (int j = 0; j < coeffs.size(); ++j) {
    price += coeffs[j] * std::pow(date, j);
  }

  return price;
}
