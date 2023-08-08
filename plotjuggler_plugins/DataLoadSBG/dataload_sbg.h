#pragma once

#include <QObject>
#include <QtPlugin>
#include <QWidget>
#include "PlotJuggler/dataloader_base.h"
#include "ui_dataload_sbg.h"

using namespace PJ;

class DataLoadSBG : public PJ::DataLoader
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.DataLoader")
  Q_INTERFACES(PJ::DataLoader)

public:
  DataLoadSBG();

  const std::vector<const char*>& compatibleFileExtensions() const override;

  bool readDataFromFile(PJ::FileLoadInfo* fileload_info,
                        PlotDataMapRef& destination) override;

  ~DataLoadSBG() override;

  const char* name() const override { return "DataLoad SBG"; }

  bool xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const override;

  bool xmlLoadState(const QDomElement& parent_element) override;

protected:
  int launchDialog(QFile& file, std::vector<std::string>* ordered_names);

private:
  std::string _default_time_axis;
  QDialog* _dialog;
  Ui::DataLoadSbg* _ui;
  QWidget* _main_win;
  PlotDataMapRef* _data_ref;
};
