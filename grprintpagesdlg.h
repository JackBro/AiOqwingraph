#ifndef GRPRINTPAGESDLG_H
#define GRPRINTPAGESDLG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
  class GRPrintPagesDlg;
}
QT_END_NAMESPACE

class GRPrintPagesDlg : public QDialog {
  Q_OBJECT
public:
  GRPrintPagesDlg(QWidget *parent = 0);
  ~GRPrintPagesDlg();

  int columns() const;
  int rows() const;

protected:
  void changeEvent(QEvent *e);

private:
  Ui::GRPrintPagesDlg *ui;
};

#endif // GRPRINTPAGESDLG_H
