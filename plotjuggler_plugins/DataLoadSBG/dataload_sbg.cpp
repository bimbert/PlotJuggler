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
#include "sbg_data_dialog.h"

DataLoadSBG::DataLoadSBG()
  : _main_win(nullptr)
  , _dialog(new QDialog())
  , _ui(new Ui::DataLoadSBG())
  , _id(0)
{
  for (QWidget* widget : qApp->topLevelWidgets()) {
    if (widget->inherits("QMainWindow")) {
      _main_win = widget;
      break;
    }
  }

  _ui->setupUi(_dialog);

  connect(_ui->sourceIdComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [=](int index){ _id = index+1; });

  _id = _ui->sourceIdComboBox->currentIndex()+1;
}

const std::vector<const char*>& DataLoadSBG::compatibleFileExtensions() const
{
  static std::vector<const char*> extensions = { "bin" };
  return extensions;
}

bool DataLoadSBG::readDataFromFile(FileLoadInfo* fileload_info,
                                   PlotDataMapRef& plot_data)
{
  QSettings settings;
  _dialog->restoreGeometry(settings.value("DataLoadSBG.geometry").toByteArray());

  int res = _dialog->exec();

  settings.setValue("DataLoadSBG.geometry", _dialog->saveGeometry());

  if (res == QDialog::Rejected) {
    return false;
  }

  const auto& filename = fileload_info->filename;

  SbgParser parser;
  parser.open(filename.toStdString());

  const auto& data = parser.data();

  // create empty timeseries
  std::vector<PlotData*> plots_vector;
  for (unsigned i = 0; i < SbgParser::DATA_MAX; i++) {
    auto it = plot_data.addNumeric(std::string("sbg")+std::to_string(_id)
                                   +"/"+SbgParser::DATA_NAME[i]);
    plots_vector.push_back(&(it->second));
  }

  // populate timeseries with data
  uint32_t ts_utc = parser.utcTimestamp();
  uint64_t ts_ref = parser.utcReference();
  for (unsigned i = 0; i < SbgParser::DATA_MAX; ++i) {
    for (const auto& it : data[i]) {
      int64_t dts = (int64_t) (it.ts - ts_utc) / 1000;
      uint64_t ts = ts_ref + dts;
      PlotData::Point point(ts*1e-3, it.val);
      plots_vector[i]->pushBack(point);
    }
  }

  // populate dialog with info
  SbgDataDialog* dialog = new SbgDataDialog(parser, _main_win);
  dialog->setWindowTitle(QString("SBG file %1").arg(filename));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->restoreSettings();
  dialog->show();

  return true;
}

DataLoadSBG::~DataLoadSBG()
{
  delete _ui;
  delete _dialog;
}

bool DataLoadSBG::xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const
{
  return true;
}

bool DataLoadSBG::xmlLoadState(const QDomElement&)
{
  return true;
}
