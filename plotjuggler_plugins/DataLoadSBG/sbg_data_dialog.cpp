#include "sbg_data_dialog.h"
#include "ui_sbg_data_dialog.h"

#include <QTableWidget>
#include <QSettings>
#include <QHeaderView>

SbgDataDialog::SbgDataDialog(const SbgParser& parser, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::SbgDataDialog)
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

void SbgDataDialog::restoreSettings()
{
  QTableWidget* table_info = ui->tableWidgetInfo;

  QSettings settings;
  restoreGeometry(settings.value("SbgDataDialog/geometry").toByteArray());
  table_info->horizontalHeader()->restoreState(
      settings.value("SbgDataDialog/info/state").toByteArray());

  table_info->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
  table_info->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
}

SbgDataDialog::~SbgDataDialog()
{
  QTableWidget* table_info = ui->tableWidgetInfo;

  QSettings settings;
  settings.setValue("SbgDataDialog/geometry", this->saveGeometry());
  settings.setValue("SbgDataDialog/info/state",
                    table_info->horizontalHeader()->saveState());

  delete ui;
}
