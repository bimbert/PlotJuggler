#pragma once

#include <QDialog>
#include <QString>
#include "usd1_parser.h"

namespace Ui
{
class Usd1DataDialog;
}

class Usd1DataDialog : public QDialog
{
  Q_OBJECT

public:
  explicit Usd1DataDialog(const Usd1Parser& parser, QWidget* parent = nullptr);

  void restoreSettings();

  ~Usd1DataDialog();

private:
  Ui::Usd1DataDialog* ui;
};
