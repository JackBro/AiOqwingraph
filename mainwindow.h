//---------------------------------------------------------------------------

#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QtGui/QMouseEvent>
#include <QtGui/QScrollArea>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QPainter>

class MouseCanvas;
struct PaintStruct;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
   MainWindow();
   
   void setZoomValue(const char *zoom);
   void setOriginValue(const char *origin);
   void setNodesValue(const char *nodes);
   void jump_focus(int x, int y);	//Added by AiO
   
   void drawGraph(PaintStruct *p);
   
   QAbstractScrollArea *canvas;

protected:

   void KeyDown(int & Key, Qt::KeyboardModifiers Modifiers);

   bool eventFilter(QObject *obj, QEvent *event);

private slots:
   void printFile();
   bool open();
   bool saveAs();
   bool close();
   
   void normal();
   void polar();
   void polarFixed();
   void cart();
   void cartFixed();
   void showLabels();
   void setShowLabelsIcon();
   
   void zoomIn();
   void zoomOut();
   void zoomNormal();
   void fitAll();
   void showSearch();		// Added by Aio
   
   void positOrigin();
   void easyPan();
   void help();
   void about();

   void scrollAction(int action);
   void scrollSliderReleased();

private:
   void createActions();
   void createMenus();
   void createToolBars();
   void createStatusBar();
   void readSettings();
   void writeSettings();
   bool maybeSave();
   bool loadFile(const QString &fileName);
   bool saveFile(const QString &fileName);
   void setCurrentFile(const QString &fileName);
   QString strippedName(const QString &fullFileName);
   
   QPlainTextEdit *textEdit;
   QString curFile;
   
   QMenu *fileMenu;
   QMenu *viewMenu;
   QMenu *zoomMenu;
   QMenu *moveMenu;
   QMenu *helpMenu;
   QToolBar *fileToolBar;
   QToolBar *zoomToolBar;
   QToolBar *viewToolBar;
   QToolBar *moveToolBar;
   
   QAction *printAct;
   QAction *openAct;
   QAction *searchAct;		//Added By AiO for search window
   QAction *saveAsAct;
   QAction *exitAct;
   
   QAction *normalAct;
   QAction *polarAct;
   QAction *polarFixedAct;
   QAction *cartAct;
   QAction *cartFixedAct;
   QAction *showLabelsAct;
   
   QAction *zoomInAct;
   QAction *zoomOutAct;
   QAction *zoomNormalAct;
   QAction *fitAllAct;
   
   QAction *positOriginAct;
   QAction *easyPanAct;
   
   QAction *helpAct;
   QAction *aboutAct;
   
   QStatusBar *myStatusBar;
   QLabel *zoom;
   QLabel *origin;
   QLabel *nodes;
   
   QScrollArea *scroller;
   QColor fgColor;
   
   QCursor *MOVING_CURSOR;
   bool sticky;
        
private:
/*   
        void FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void vbarScroll(TObject *Sender, TScrollCode ScrollCode,
          int &ScrollPos);
        void hbarScroll(TObject *Sender, TScrollCode ScrollCode,
          int &ScrollPos);
        void FormMouseWheel(TObject *Sender, TShiftState Shift,
          int WheelDelta, TPoint &MousePos, bool &Handled);
*/
private:	// User declarations
   int xpages, ypages;
};

struct PaintStruct {
   QPainter *p;
   unsigned int border;
};

#endif

