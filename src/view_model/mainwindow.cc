#include "mainwindow.h"

#include "../view/ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow),
      model_(new StockForecaster()) {
  ui_->setupUi(this);
  HideLegend(ui_->ipnLegend);
  HideLegend(ui_->apnLegend);
  InitPlot(ui_->ipnPlot);
  InitPlot(ui_->apnPlot);
}

MainWindow::~MainWindow() {
  delete ui_;
  delete model_;
}

void MainWindow::on_loadDataBtn_clicked() {
  QString filename =
      QFileDialog::getOpenFileName(this, "Load data", ".", "*.csv");
  if (!filename.isEmpty()) {
    if (model_->LoadData(filename.toStdString())) {
      QMessageBox::information(this, "Notice", "Data loaded successfully",
                               QMessageBox::Ok);
      ui_->fileNameLabel->setText(filename);
      InitControlPanel();
      on_apnClearCanvasBtn_clicked();
      on_ipnClearCanvasBtn_clicked();
    } else {
      QMessageBox::critical(this, "Error",
                            QString::fromStdString(model_->GetError()),
                            QMessageBox::Ok);
    }
  }
}

void MainWindow::on_ipnDrawGraphBtn_clicked() {
  if (model_->InterpolatePricesByCubicSplineMethod(
          ui_->ipnPointsCountSpinBox->value())) {
    DrawInterpolationGraph();
    if (ui_->ipnPlot->graphCount() == kMaxPlotsCount) {
      ui_->ipnDrawGraphBtn->setDisabled(true);
    }
  } else {
    QMessageBox::critical(this, "Error",
                          QString::fromStdString(model_->GetError()),
                          QMessageBox::Ok);
  }
}

void MainWindow::on_apnDrawGraphBtn_clicked() {
  if (model_->ApproximatePricesByLeastSquaresMethod(
          ui_->apnPointsCountSpinBox->value(),
          ui_->apnDaysCountSpinBox->value(),
          ui_->apnPolyDegreeSpinBox->value())) {
    DrawApproximationGraph();
    if (ui_->apnPlot->graphCount() == kMaxPlotsCount + 1) {
      ui_->apnDrawGraphBtn->setDisabled(true);
    }
  } else {
    QMessageBox::critical(this, "Error",
                          QString::fromStdString(model_->GetError()),
                          QMessageBox::Ok);
  }
}

void MainWindow::on_ipnClearCanvasBtn_clicked() {
  ui_->ipnPlot->clearGraphs();
  HideLegend(ui_->ipnLegend);
  ui_->ipnDrawGraphBtn->setEnabled(true);
  ui_->ipnPointsCountSpinBox->setValue(0);
  ui_->ipnPlot->replot();
}

void MainWindow::on_apnClearCanvasBtn_clicked() {
  ui_->apnPlot->clearGraphs();
  HideLegend(ui_->apnLegend);
  ui_->apnDrawGraphBtn->setEnabled(true);
  ui_->apnPointsCountSpinBox->setValue(0);
  ui_->apnPlot->replot();
}

void MainWindow::on_ipnForecastBtn_clicked() {
  if (model_->InterpolatePriceByCubicSplineMethod(
          ui_->ipnDateBox->dateTime().toSecsSinceEpoch())) {
    ui_->ipnForecastPriceBox->setValue(model_->GetForecastPrice());
  } else {
    QMessageBox::critical(this, "Error",
                          QString::fromStdString(model_->GetError()),
                          QMessageBox::Ok);
  }
}

void MainWindow::on_apnForecastBtn_clicked() {
  if (model_->ApproximatePriceByLeastSquaresMethod(
          ui_->apnDateBox->dateTime().toSecsSinceEpoch(),
          ui_->apnPolyDegreeSpinBox->value())) {
    ui_->apnForecastPriceBox->setValue(model_->GetForecastPrice());
  } else {
    QMessageBox::critical(this, "Error",
                          QString::fromStdString(model_->GetError()),
                          QMessageBox::Ok);
  }
}

void MainWindow::InitPlot(QCustomPlot* plot) {
  plot->xAxis->setLabel("Date");
  plot->xAxis->setLabelFont(QFont(QFont().family(), 14, QFont::Bold));
  plot->yAxis->setLabel("Price");
  plot->yAxis->setLabelFont(QFont(QFont().family(), 14, QFont::Bold));

  QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
  dateTicker->setDateTimeFormat("d MMMM\nyyyy");
  plot->xAxis->setTicker(dateTicker);
  plot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));

  QDateTime now = QDateTime::currentDateTime();
  plot->xAxis->setRange(now.toSecsSinceEpoch(),
                        now.addDays(5).toSecsSinceEpoch());

  // allow user to drag axis ranges with mouse, zoom with mouse wheel
  plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  plot->replot();
}

void MainWindow::InitControlPanel() {
  ui_->ipnPointsCountSpinBox->setMinimum(model_->GetData().size());
  ui_->ipnDateBox->setMaximumDateTime(
      QDateTime::fromSecsSinceEpoch(model_->GetMaxDate()));
  ui_->ipnDateBox->setMinimumDateTime(
      QDateTime::fromSecsSinceEpoch(model_->GetMinDate()));
  ui_->ipnForecastPriceBox->setValue(0);

  ui_->apnPointsCountSpinBox->setMinimum(model_->GetData().size());
  ui_->apnDateBox->setMaximumDateTime(
      QDateTime::fromSecsSinceEpoch(model_->GetMaxDate())
          .addDays(ui_->apnDaysCountSpinBox->maximum()));
  ui_->apnDateBox->setMinimumDateTime(
      QDateTime::fromSecsSinceEpoch(model_->GetMinDate()));
  ui_->apnForecastPriceBox->setValue(0);
}

void MainWindow::DrawInterpolationGraph() {
  ui_->ipnPlot->addGraph();
  PrepareGraph(ui_->ipnPlot, ui_->ipnPlot->graph(),
               ui_->ipnPlot->graphCount() + 6);
  ShowLegendItem(ui_->ipnLegend, ui_->ipnPlot->graphCount());
  ui_->ipnPlot->replot();
}

void MainWindow::DrawApproximationGraph() {
  if (ui_->apnPlot->graphCount() == 0) {
    ui_->apnPlot->addGraph();
    PrepareDataSet(ui_->apnPlot, ui_->apnPlot->graph());
  }

  ui_->apnPlot->addGraph();
  PrepareGraph(ui_->apnPlot, ui_->apnPlot->graph(),
               ui_->apnPlot->graphCount() + 5);
  ShowLegendItem(ui_->apnLegend, ui_->apnPlot->graphCount() - 1);
  ui_->apnPlot->replot();
}

void MainWindow::PrepareDataSet(QCustomPlot* plot, QCPGraph* graph) {
  // set dots style
  graph->setLineStyle(QCPGraph::lsNone);

  QCPScatterStyle scatterStyle;
  scatterStyle.setShape(
      QCPScatterStyle::ssDiamond);   // Set the shape of the scatter points
  scatterStyle.setPen(Qt::NoPen);    // No outline for the points
  scatterStyle.setBrush(Qt::black);  // Set the fill color to black
  scatterStyle.setSize(8);           // Set the size of the scatter points
  graph->setScatterStyle(scatterStyle);

  // set data
  QVector<double> dates, prices;
  for (auto& data_point : model_->GetData()) {
    dates.push_back(data_point.date.ToDouble());
    prices.push_back(data_point.price);
  }
  graph->setData(dates, prices);

  // set axis ranges to show all data
  graph->rescaleAxes();
  plot->yAxis->setRangeLower(0);
  double upper_bound = plot->yAxis->range().upper;
  plot->yAxis->setRangeUpper(upper_bound + upper_bound * 0.2);
}

void MainWindow::PrepareGraph(QCustomPlot* plot, QCPGraph* graph,
                              int color_num) {
  // set line style
  graph->setLineStyle(QCPGraph::lsLine);
  graph->setPen(QPen(Qt::GlobalColor(color_num), 2, Qt::SolidLine,
                     Qt::SquareCap, Qt::MiterJoin));

  // set data
  QVector<double> dates, prices;
  for (auto& data_point : model_->GetForecast()) {
    dates.push_back(data_point.date.ToDouble());
    prices.push_back(data_point.price);
  }
  graph->setData(dates, prices);

  // set axis ranges to show all data
  if (plot->graphCount() == 1) {
    graph->rescaleAxes();

    plot->yAxis->setRangeLower(0);
    double upper_bound = plot->yAxis->range().upper;
    plot->yAxis->setRangeUpper(upper_bound + upper_bound * 0.2);
  } else {
    graph->rescaleAxes(true);
  }
}

void MainWindow::ShowLegendItem(QWidget* legend, int item_position) {
  // find legend item
  QWidget* legend_item =
      static_cast<QWidget*>(legend->children().at(item_position));

  // check checkBox
  QCheckBox* check_box =
      legend_item
          ->findChildren<QCheckBox*>(QRegularExpression("\\w{1,}CheckBox$"))
          .first();
  check_box->setCheckState(Qt::Checked);

  // set graph's name
  QLabel* graph_name =
      legend_item->findChildren<QLabel*>(QRegularExpression("\\w{1,}Label$"))
          .first();
  graph_name->setText(QString::number(model_->GetForecast().size()) + " pts");

  // show item
  legend_item->setVisible(true);
}

void MainWindow::HideLegend(QWidget* legend) {
  for (auto child : legend->children()) {
    static_cast<QWidget*>(child)->setVisible(false);
  }
}

void MainWindow::ChangeGraphVisibility(QCustomPlot* plot, int graph_number,
                                       bool visible) {
  plot->graph(graph_number)->setVisible(visible);
  plot->replot();
}

void MainWindow::on_ipnLegendRedCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->ipnPlot, 0, state == Qt::Checked);
}

void MainWindow::on_ipnLegendGreenCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->ipnPlot, 1, state == Qt::Checked);
}

void MainWindow::on_ipnLegendBlueCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->ipnPlot, 2, state == Qt::Checked);
}

void MainWindow::on_ipnLegendCyanCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->ipnPlot, 3, state == Qt::Checked);
}

void MainWindow::on_ipnLegendMagentaCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->ipnPlot, 4, state == Qt::Checked);
}

void MainWindow::on_apnLegendRedCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->apnPlot, 1, state == Qt::Checked);
}

void MainWindow::on_apnLegendGreenCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->apnPlot, 2, state == Qt::Checked);
}

void MainWindow::on_apnLegendBlueCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->apnPlot, 3, state == Qt::Checked);
}

void MainWindow::on_apnLegendCyanCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->apnPlot, 4, state == Qt::Checked);
}

void MainWindow::on_apnLegendMagentaCheckBox_stateChanged(int state) {
  ChangeGraphVisibility(ui_->apnPlot, 5, state == Qt::Checked);
}

void MainWindow::on_apnDaysCountSpinBox_valueChanged() {
  on_apnClearCanvasBtn_clicked();
}
