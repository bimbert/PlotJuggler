#include "dataload_neo.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QWidget>
#include <QSettings>
#include <QProgressDialog>
#include <QMainWindow>
#include "neo_parser.h"
#include "neo_data_dialog.h"

DataLoadNeo::DataLoadNeo()
  : _main_win(nullptr)
{
  for (QWidget* widget : qApp->topLevelWidgets()) {
    if (widget->inherits("QMainWindow")) {
      _main_win = widget;
      break;
    }
  }
}

const std::vector<const char*>& DataLoadNeo::compatibleFileExtensions() const
{
  static std::vector<const char*> extensions = { "neobin" };
  return extensions;
}

bool DataLoadNeo::readDataFromFile(FileLoadInfo* fileload_info,
                                   PlotDataMapRef& plot_data)
{
  const auto& filename = fileload_info->filename;

  NeoParser parser;
  parser.parse(filename.toStdString());

  const auto& data = parser.data();

  // create empty timeseries
  std::vector<PlotData*> plots_vector;
  for (unsigned i = 0; i < NeoParser::DATA_MAX; i++) {
    auto it = plot_data.addNumeric(std::string("neo")+"/"+NeoParser::DATA_NAME[i]);
    plots_vector.push_back(&(it->second));
  }

  // populate timeseries with data
  for (unsigned i = 0; i < NeoParser::DATA_MAX; ++i) {
    for (const auto& it : data[i]) {
      PlotData::Point point(it.ts*1e-6, it.val);
      plots_vector[i]->pushBack(point);
    }
  }

  // populate dialog with info
  NeoDataDialog* dialog = new NeoDataDialog(parser, _main_win);
  dialog->setWindowTitle(QString("NEO file %1").arg(filename));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->restoreSettings();
  dialog->show();

  return true;
}

DataLoadNeo::~DataLoadNeo()
{
}

bool DataLoadNeo::xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const
{
  return true;
}

bool DataLoadNeo::xmlLoadState(const QDomElement&)
{
  return true;
}
