#ifndef ALGORITHMIC_TRADING_MAINWINDOW_H
#define ALGORITHMIC_TRADING_MAINWINDOW_H

#include <QMainWindow>

#include "../../libs/qcustomplot.h"
#include "../model/stockforecaster.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private slots:
  void on_loadDataBtn_clicked();

  // Interpolation
  void on_ipnDrawGraphBtn_clicked();
  void on_ipnClearCanvasBtn_clicked();
  void on_ipnForecastBtn_clicked();
  void on_ipnLegendRedCheckBox_stateChanged(int state);
  void on_ipnLegendGreenCheckBox_stateChanged(int state);
  void on_ipnLegendBlueCheckBox_stateChanged(int state);
  void on_ipnLegendCyanCheckBox_stateChanged(int state);
  void on_ipnLegendMagentaCheckBox_stateChanged(int state);

  // Approximation
  void on_apnClearCanvasBtn_clicked();
  void on_apnDrawGraphBtn_clicked();
  void on_apnForecastBtn_clicked();
  void on_apnLegendMagentaCheckBox_stateChanged(int state);
  void on_apnLegendCyanCheckBox_stateChanged(int state);
  void on_apnLegendBlueCheckBox_stateChanged(int state);
  void on_apnLegendGreenCheckBox_stateChanged(int state);
  void on_apnLegendRedCheckBox_stateChanged(int state);
  void on_apnDaysCountSpinBox_valueChanged();

 private:
  const int kMaxPlotsCount = 5;

  void InitPlot(QCustomPlot* plot);
  void InitControlPanel();
  void DrawInterpolationGraph();
  void DrawApproximationGraph();
  void PrepareGraph(QCustomPlot* plot, QCPGraph* graph, int colorNum);
  void PrepareDataSet(QCustomPlot* plot, QCPGraph* graph);
  void ShowLegendItem(QWidget* legend, int item_position);
  void HideLegend(QWidget* legend);
  void ChangeGraphVisibility(QCustomPlot* plot, int graph_number, bool visible);

  Ui::MainWindow* ui_;
  StockForecaster* model_;
};
#endif  // ALGORITHMIC_TRADING_MAINWINDOW_H
