#include "dataload_sbg.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QWidget>
#include <QSettings>
#include <QProgressDialog>
#include <QMainWindow>
#include "sbg_parser.h"
//#include "sbg_data_dialog.h"

DataLoadSBG::DataLoadSBG()
  : _main_win(nullptr)
{
  for (QWidget* widget : qApp->topLevelWidgets()) {
    if (widget->inherits("QMainWindow")) {
      _main_win = widget;
      break;
    }
  }
}

const std::vector<const char*>& DataLoadSBG::compatibleFileExtensions() const
{
  static std::vector<const char*> extensions = { "bin" };
  return extensions;
}

bool DataLoadSBG::readDataFromFile(FileLoadInfo* fileload_info,
                                   PlotDataMapRef& plot_data)
{
  const auto& filename = fileload_info->filename;

  SbgParser parser;
  parser.open(filename.toStdString());

  const auto& data = parser.data();

  // create empty timeseries
  std::vector<PlotData*> plots_vector;
  for (unsigned i = 0; i < SbgParser::DATA_MAX; i++) {
    auto it = plot_data.addNumeric(SbgParser::DATA_NAME[i]);
    plots_vector.push_back(&(it->second));
  }

  // populate timeseries with data
  for (unsigned i = 0; i < SbgParser::DATA_MAX; ++i) {
    for (const auto& it : data[i]) {
      PlotData::Point point(it.ts*1e-6, it.val);
      plots_vector[i]->pushBack(point);
    }
  }

//  // populate dialog with info
//  SbgDataDialog* dialog = new SbgDataDialog(parser, _main_win);
//  dialog->setWindowTitle(QString("SBG file %1").arg(filename));
//  dialog->setAttribute(Qt::WA_DeleteOnClose);
//  dialog->restoreSettings();
//  dialog->show();

  return true;
}

DataLoadSBG::~DataLoadSBG()
{
}

bool DataLoadSBG::xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const
{
  return true;
}

bool DataLoadSBG::xmlLoadState(const QDomElement&)
{
  return true;
}
