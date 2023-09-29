#pragma once

#include <QObject>
#include <QtPlugin>
#include <QWidget>
#include "PlotJuggler/dataloader_base.h"
#include "ui_dataload_zr.h"

using namespace PJ;

class DataLoadZR : public PJ::DataLoader
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.DataLoader")
  Q_INTERFACES(PJ::DataLoader)

public:
  DataLoadZR();

  const std::vector<const char*>& compatibleFileExtensions() const override;

  bool readDataFromFile(PJ::FileLoadInfo* fileload_info,
                        PlotDataMapRef& destination) override;

  ~DataLoadZR() override;

  const char* name() const override
  {
    return "DataLoad ZR";
  }

  bool xmlSaveState(QDomDocument& doc, QDomElement& parent_element) const override;

  bool xmlLoadState(const QDomElement& parent_element) override;

private:
  std::string _default_time_axis;
  QWidget* _main_win;

  QDialog* _dialog;
  Ui::DataLoadZR* _ui;

  enum type_t { NEO, US_D1};
  type_t _type;
};
