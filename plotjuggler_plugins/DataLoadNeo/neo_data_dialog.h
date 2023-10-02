#pragma once

#include <QDialog>
#include <QString>
#include "neo_parser.h"

namespace Ui
{
class NeoDataDialog;
}

class NeoDataDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeoDataDialog(const NeoParser& parser, QWidget* parent = nullptr);

  void restoreSettings();

  ~NeoDataDialog();

private:
  Ui::NeoDataDialog* ui;
};
