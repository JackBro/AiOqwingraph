#include "grprintpagesdlg.h"
#include "ui_grprintpagesdlg.h"

GRPrintPagesDlg::GRPrintPagesDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GRPrintPagesDlg)
{
  ui->setupUi(this);

  ui->mainLayout->setMargin(10);
  setLayout(ui->mainLayout);

  QStringList pages;
  for ( int i = 1; i <= 9; i++ )
    pages.append(QString::number(i));

  ui->comboColumns->addItems(pages);
  ui->comboRows->addItems(pages);

  ui->comboColumns->setCurrentIndex(0);
  ui->comboRows->setCurrentIndex(0);

  connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

GRPrintPagesDlg::~GRPrintPagesDlg()
{
  delete ui;
}

void GRPrintPagesDlg::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

int GRPrintPagesDlg::columns() const
{
  return ui->comboColumns->currentIndex() + 1;
}

int GRPrintPagesDlg::rows() const
{
  return ui->comboRows->currentIndex() + 1;
}
