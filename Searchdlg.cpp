/*
*			Searchdlg.cpp
*	AiO
*   0222'16
*   Qt based search dialog for QWinGraph for Ida
*		search for node and double click on results
*/

#include "Searchdlg.h"
#include "ui_SearchDialog.h"

#include <QHash>
#include <QString>
#include <QStringList>



Search_Dialog::Search_Dialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Search_Dialog)
{
	ui->setupUi(this);
	
	
	ui->RegEx_CB->setChecked(false);
	ui->Case_CB->setChecked(false);
	
	
	connect(ui->SearchButton, SIGNAL(clicked()), this, SLOT(search()));
	connect(ui->listResults, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(show_selected(QListWidgetItem *)));
	connect(ui->SearchEdit, SIGNAL(returnPressed()), this, SLOT(search()));
	ui->SearchEdit->setFocus();

}


Search_Dialog::~Search_Dialog()
{
	delete ui;
}

void Search_Dialog::search()
{
	ui->label->setText( tr("Search Started"));
	
	
	
	//object for found
	if(ui->listResults->count() > 0)
	{
		//clear list
		ui->listResults->clear();
	}
	//get listbox size if not 0 clear
	
	// clear matched_gnode
	if(!SearchResults.isEmpty())
	{
		//empty list
		SearchResults.clear();
	}
	
	if(!get_node_list())
	{
		//show error msg
		ui->label->setText( tr("get_node_list Eror"));
		return;
	}
	searchString = ui->SearchEdit->text();
	if(ui->RegEx_CB->isChecked())
	{
		//edit box value test for not empty msg if 0
		QRegExp rx(searchString);
		//loop through node_list
		QHash<QString,QPoint>::const_iterator i = MyNodeHash.constBegin();
		while(i != MyNodeHash.constEnd())
		{
			//search current item
		
			//if match add no found_list
			if(!rx.indexIn(i.key()))
			{
				SearchResults.append(i.key()); //copy key to list
			}
			i++;
		}
	}
	else
	{
		if(ui->Case_CB->isChecked())
		{
			QHashIterator<QString,QPoint> i(MyNodeHash);
			while(i.hasNext())
			{
				i.next();
				if(i.key().contains(searchString))
				{
					SearchResults.append(i.key()); //copy key to list
				}
			}
		}
		else
		{
			QHashIterator<QString, QPoint> i(MyNodeHash);
			while(i.hasNext())
			{
				i.next();
				if(i.key().contains(searchString,Qt::CaseInsensitive))
				{
					SearchResults.append(i.key()); //copy key to list
				}
			}
		}
		
	}
	//if found anything populate the list box
	if(!SearchResults.isEmpty())
	{
		for(int i=0; i < SearchResults.size(); ++i)
		{
			ui->listResults->addItem(  SearchResults.at(i) );
		}
		
	}
	ui->label->setText( tr("found %1").arg(SearchResults.count()));
	return;
}




// Generate a list of node labels with the color markers removed
bool Search_Dialog::get_node_list()
{
	long tmpX, tmpY;
	QPoint p;
	QString tmpLabel;
	char *pLabel;
	
	ui->label->setText( tr("Building Node List"));
	//Test for current list for use in multiple search
	if(!MyNodeHash.isEmpty()) return true; //update
	//walk through all nodes
	init_node_iterator();
	do
	{
		//
		if(get_current_node_error() == 1) break;
		pLabel = get_current_node_label();
		if(get_current_node_error())
		{
			// handle error
			
			return false;
		}
		tmpX = get_current_node_x();
		if(get_current_node_error())
		{
			// handle error
			return false;
		}
		tmpY = get_current_node_y();
		if(get_current_node_error())
		{
			// handle error
			return false;
		}
		if(pLabel == 0) break;
		tmpLabel = QString(pLabel);
		p = QPoint(tmpX, tmpY);
		
		// add to object
		MyNodeHash[tmpLabel] = p;
	}while( next_node());
	
	return true;
}

//Connected to signal	itemDoubleClicked
bool Search_Dialog::show_selected(QListWidgetItem * item)
{
	ui->label->setText(item->text());
	//take item name and use as key for MyNodeHash
	if(! MyNodeHash.contains(item->text())) return false;
	
	if(ox == MyNodeHash.value( item->text()).x() && oy == MyNodeHash.value( item->text()).y()) return true;
	
	// update position
	
	Form1->jump_focus( MyNodeHash.value( item->text()).x() , MyNodeHash.value( item->text()).y() );
	
	
	Form1->canvas->viewport()->update();
	ui->label->setText( tr("set x= %1, y= %2").arg(MyNodeHash.value(item->text()).x()).arg(MyNodeHash.value( item->text()).y()));
	return true;
}

