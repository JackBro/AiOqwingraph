/*
*			Searchdlg.h
*	AiO
*   0222'16
*   Qt based search dialog for QWinGraph for Ida
*		search for node and double click on results
*/




#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include <QDialog>
#include <QRegExp>
#include <QPoint>
#include <QHash>
#include <QListWidgetItem>
#include "mainwindow.h"

extern "C"
{
	int init_node_iterator();
	int next_node();
	int get_current_node_error();
	char *get_current_node_label();
	int get_current_node_x();
	int get_current_node_y();
}


	extern int ox;
	extern int oy;
	extern MainWindow *Form1;
 
QT_BEGIN_NAMESPACE
namespace Ui {
	class Search_Dialog;
}
QT_END_NAMESPACE



class Search_Dialog : public QDialog {
	Q_OBJECT
public:
	Search_Dialog(QWidget *parent = 0);
	~Search_Dialog();


	
protected:
	
	void regex_selected();

	
	//data structure for list of nodes
	bool get_node_list();
	
private:
	Ui::Search_Dialog *ui;
	QHash<QString,QPoint> MyNodeHash;	//data structure to hold node labels and point{x,y}
	QStringList SearchResults;
	QString searchString;
private slots:
	bool show_selected(QListWidgetItem * item);
	void search();
};




#endif // SEARCHDLG_H
