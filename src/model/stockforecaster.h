#ifndef ALGORITHMIC_TRADING_MODEL_STOCKFORECASTER_H
#define ALGORITHMIC_TRADING_MODEL_STOCKFORECASTER_H

#include <array>
#include <chrono>
#include <vector>

#include "data_point.h"

class StockForecaster {
  using Matrix = std::vector<std::vector<double>>;

  enum CubicInterpolationCoefficients { A, B, C, D };

 public:
  bool LoadData(const std::string& file_path);

  bool InterpolatePriceByCubicSplineMethod(time_t date);
  bool InterpolatePricesByCubicSplineMethod(int dates_count);

  bool ApproximatePriceByLeastSquaresMethod(time_t date, int degree);
  bool ApproximatePricesByLeastSquaresMethod(int dates_count, int future_days,
                                             int degree);

  time_t GetMaxDate() const;
  time_t GetMinDate() const;

  const std::string& GetError() const;
  double GetForecastPrice() const;
  const std::vector<DataPoint>& GetForecast() const;
  const std::vector<DataPoint>& GetData() const;

 private:
  // common
  std::vector<time_t> DefineDates(int dates_count, time_t period);
  std::vector<double> SolveSle(Matrix& sle);

  // Interpolation
  std::array<std::vector<double>, 4> DefineInterpolationCoefficients();
  int DefinePivotDateIndex(time_t date);
  double InterpolatePrice(time_t date,
                          const std::array<std::vector<double>, 4>& coeffs,
                          int pivot_date_idx);

  // Approximation
  std::vector<double> DefineApproximationCoefficients(int degree);
  double ApproximatePrice(time_t date, std::vector<double> coeffs);

  std::string error_message_;
  double forecast_price_ = 0.0;
  std::vector<DataPoint> forecast_;
  std::vector<DataPoint> data_;
};

#endif  // ALGORITHMIC_TRADING_MODEL_STOCKFORECASTER_H
