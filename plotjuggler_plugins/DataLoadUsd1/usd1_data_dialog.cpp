#include "usd1_data_dialog.h"
#include "ui_usd1_data_dialog.h"

#include <QTableWidget>
#include <QSettings>
#include <QHeaderView>

Usd1DataDialog::Usd1DataDialog(const Usd1Parser& parser, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::Usd1DataDialog)
{
  ui->setupUi(this);
  QTableWidget* table_info = ui->tableWidgetInfo;

  table_info->setRowCount((int)parser.info().size());
  int row = 0;
  for (const auto& it : parser.info())
  {
    table_info->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(it.first)));
    table_info->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(it.second)));
    row++;
  }
  table_info->sortItems(0);
}

void Usd1DataDialog::restoreSettings()
{
  QTableWidget* table_info = ui->tableWidgetInfo;

  QSettings settings;
  restoreGeometry(settings.value("Usd1DataDialog/geometry").toByteArray());
  table_info->horizontalHeader()->restoreState(
      settings.value("Usd1DataDialog/info/state").toByteArray());

  table_info->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
  table_info->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
}

Usd1DataDialog::~Usd1DataDialog()
{
  QTableWidget* table_info = ui->tableWidgetInfo;

  QSettings settings;
  settings.setValue("Usd1DataDialog/geometry", this->saveGeometry());
  settings.setValue("Usd1DataDialog/info/state",
                    table_info->horizontalHeader()->saveState());

  delete ui;
}
