#pragma once

#include <QDialog>
#include <QString>
#include "sbg_parser.h"

namespace Ui
{
class SbgDataDialog;
}

class SbgDataDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SbgDataDialog(const SbgParser& parser, QWidget* parent = nullptr);

  void restoreSettings();

  ~SbgDataDialog();

private:
  Ui::SbgDataDialog* ui;
};
