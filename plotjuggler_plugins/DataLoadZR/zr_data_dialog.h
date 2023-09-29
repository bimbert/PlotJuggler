#pragma once

#include <QDialog>
#include <QString>
#include "zr_parser.h"

namespace Ui
{
class ZrDataDialog;
}

class ZrDataDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZrDataDialog(const ZrParser& parser, QWidget* parent = nullptr);

  void restoreSettings();

  ~ZrDataDialog();

private:
  Ui::ZrDataDialog* ui;
};
