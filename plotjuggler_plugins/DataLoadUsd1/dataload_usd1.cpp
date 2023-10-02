#include "dataload_usd1.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QWidget>
#include <QSettings>
#include <QProgressDialog>
#include <QMainWindow>
#include "usd1_parser.h"
#include "usd1_data_dialog.h"

DataLoadUsd1::DataLoadUsd1()
  : _main_win(nullptr)
{
  for (QWidget* widget : qApp->topLevelWidgets()) {
    if (widget->inherits("QMainWindow")) {
      _main_win = widget;
      break;
    }
  }
}

const std::vector<const char*>& DataLoadUsd1::compatibleFileExtensions() const
{
  static std::vector<const char*> extensions = { "usd1bin" };
  return extensions;
}

bool DataLoadUsd1::readDataFromFile(FileLoadInfo* fileload_info,
                                   PlotDataMapRef& plot_data)
{
  const auto& filename = fileload_info->filename;

  Usd1Parser parser;
  parser.parse(filename.toStdString());

  const auto& data = parser.data();

  // create empty timeseries
  std::vector<PlotData*> plots_vector;
  for (unsigned i = 0; i < Usd1Parser::DATA_MAX; i++) {
    auto it = plot_data.addNumeric(std::string("usd1")+"/"+Usd1Parser::DATA_NAME[i]);
    plots_vector.push_back(&(it->second));
  }

  // populate timeseries with data
  for (unsigned i = 0; i < Usd1Parser::DATA_MAX; ++i) {
    for (const auto& it : data[i]) {
      PlotData::Point point(it.ts*1e-6, it.val);
      plots_vector[i]->pushBack(point);
    }
  }

  // populate dialog with info
  Usd1DataDialog* dialog = new Usd1DataDialog(parser, _main_win);
  dialog->setWindowTitle(QString("US-D1 file %1").arg(filename));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->restoreSettings();
  dialog->show();

  return true;
}

DataLoadUsd1::~DataLoadUsd1()
{
}

bool DataLoadUsd1::xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const
{
  return true;
}

bool DataLoadUsd1::xmlLoadState(const QDomElement&)
{
  return true;
}
