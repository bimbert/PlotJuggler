#include "dataload_zr.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QWidget>
#include <QSettings>
#include <QProgressDialog>
#include <QMainWindow>
#include "zr_parser.h"
#include "zr_data_dialog.h"

DataLoadZR::DataLoadZR()
  : _main_win(nullptr)
  , _dialog(new QDialog())
  , _ui(new Ui::DataLoadZR())
  , _type(NEO)
{
  for (QWidget* widget : qApp->topLevelWidgets()) {
    if (widget->inherits("QMainWindow")) {
      _main_win = widget;
      break;
    }
  }

  _ui->setupUi(_dialog);

  connect(_ui->sourceIdComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [=](int index){ _type = (type_t)index; });

  _type = (type_t)_ui->sourceIdComboBox->currentIndex();
}

const std::vector<const char*>& DataLoadZR::compatibleFileExtensions() const
{
  static std::vector<const char*> extensions = { "zrbin" };
  return extensions;
}

bool DataLoadZR::readDataFromFile(FileLoadInfo* fileload_info,
                                   PlotDataMapRef& plot_data)
{
  QSettings settings;
  _dialog->restoreGeometry(settings.value("DataLoadZR.geometry").toByteArray());

  int res = _dialog->exec();

  settings.setValue("DataLoadZR.geometry", _dialog->saveGeometry());

  if (res == QDialog::Rejected) {
    return false;
  }

  const auto& filename = fileload_info->filename;

  ZrParser parser;
  parser.open(filename.toStdString());

  const auto& data = parser.data();

  // create empty timeseries
  std::vector<PlotData*> plots_vector;
  for (unsigned i = 0; i < ZrParser::DATA_MAX; i++) {
    auto it = plot_data.addNumeric(std::string("zr")+std::to_string(_id)
                                   +"/"+ZrParser::DATA_NAME[i]);
    plots_vector.push_back(&(it->second));
  }

  // populate timeseries with data
  for (unsigned i = 0; i < ZrParser::DATA_MAX; ++i) {
    for (const auto& it : data[i]) {
      PlotData::Point point(it.ts*1e-6, it.val);
      plots_vector[i]->pushBack(point);
    }
  }

  // populate dialog with info
  ZrDataDialog* dialog = new ZrDataDialog(parser, _main_win);
  dialog->setWindowTitle(QString("ZR file %1").arg(filename));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->restoreSettings();
  dialog->show();

  return true;
}

DataLoadZR::~DataLoadZR()
{
  delete _ui;
  delete _dialog;
}

bool DataLoadZR::xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const
{
  return true;
}

bool DataLoadZR::xmlLoadState(const QDomElement&)
{
  return true;
}
