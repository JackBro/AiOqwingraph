/*
 *      WinGraph32 - Graph Visualization Program
 *      Version 1.0
 *      The WIN32 interface written by Ilfak Guilfanov. (ig@datarescue.com)
 *
 *      This program is under GPL (GNU General Public License)
 *      It is based on the VCG tool written by Georg Sander and Iris Lemke
 *
 *      Version 1.01
 *      - printing
 *      - fixed a bug in step4.c
 *      Version 1.02
 *      - 256 color entries are supported
 *		
 *		Version AiO 0.1
 *		Adding Open dialog
 *		Adding search function
 *		Adding right click node to copy label to clipboard
 *
 */

//---------------------------------------------------------------------------

#include <QtGui/QtGui>
#include <QtCore/QPoint>
#include <QtGui/QMessageBox>
#include <QtGui/QClipboard>
#include <QApplication>

#include "mainwindow.h"
#include "grprintpagesdlg.h"
#include "Searchdlg.h"


#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#define _snprintf snprintf
#endif

struct Point {
  int x;
  int y;
};

//---------------------------------------------------------------------------

MainWindow *Form1;

extern "C" {
	//#include "alloc.h"  //errors galore
  void statistics(void);
  extern int st_nr_vis_nodes, st_nr_vis_edges, nr_crossings;
  extern const unsigned char redmap[];
  extern const unsigned char greenmap[];
  extern const unsigned char bluemap[];
  int vcg_main(int argc, char *argv[]);
  void draw_graph(void (*_line_cb)(int x1,int y1,int x2,int y2,int color, void *painter),
                  void (*_rectangle_cb)(long x,long y,int w,int h,int color, void *painter),
                  void (*_polygon_cb)(Point *hp, int j, int color, void *painter),
                  void *painter);
  void relayout(void);
  void m_validate_fe(int code);
  void display_complete_graph(void);

  bool set_drawing_rectangle(int width, int height);
  void m_reload(void);
  void parse_part(void);
  void visualize_part(void);
  void error(const char *format, ...);
  //   void cleanup_streams(void);
  void set_fe_scaling(int stretch, int shrink);
  void change_fe_scaling(int stretch, int shrink);
  void visualize_part(void);
  int incr_fe_focus(long dx, long dy);
  int set_node_focus(long x, long y);			//Added
  
  int normal_fe_focus(void);
  void save_input_file_contents(FILE *fp);
  extern char *G_title;
  extern int G_color;
  extern int G_displayel;
  extern int G_stretch, G_shrink;
  extern long V_xmin, V_ymin;
  extern long fe_scaling;
  extern long gfocus_x, gfocus_y;
  extern int fisheye_view;
  extern char Dataname[];	//used to set file name for parser
  // Add by AiO to get char ptr
  
  char* node_xy_title(long x, long y);
}
//Moved and removed static
int ox, oy;

//---------------------------------------------------------------------------
static void update_origin_status(void) {
  char buf[80];
  if (fisheye_view != 0) {
    _snprintf(buf, sizeof(buf), "(%ld,%ld)", gfocus_x, gfocus_y);
  }
  else {
    _snprintf(buf, sizeof(buf), "(%ld,%ld)", -V_xmin, -V_ymin);
  }
  Form1->setOriginValue(buf);
}

void MainWindow::setOriginValue(const char *_origin) {
  origin->setText(_origin);
}

//---------------------------------------------------------------------------
static void update_zoom_status(void) {
  char buf[80];
  double zoom;
  if (fisheye_view != 0) {
    zoom = fe_scaling;
  }
  else {
    zoom = double(G_stretch) * 100;
    if ( G_shrink != 0 ) zoom /= G_shrink;
  }
  _snprintf(buf, sizeof(buf), "%5.2f%%", zoom);
  Form1->setZoomValue(buf);
  update_origin_status();
}

void MainWindow::setZoomValue(const char *_zoom) {
  zoom->setText(_zoom);
}

//---------------------------------------------------------------------------
void move_focus(int dx, int dy) {
  incr_fe_focus(dx, dy);
  update_origin_status();
}

//--------------------------------------------------------------------------
//  Added by AiO to jump to position
//--------------------------------------------------------------------------
void MainWindow::jump_focus(int x, int y) {
	Form1->positOrigin();
	set_node_focus(x,y);
	
	update_origin_status();
}

//---------------------------------------------------------------------------
static QColor vcg2bcc(int color) {
  if ( color < 256 ) {
    int r = redmap[color];
    int g = greenmap[color];
    int b = bluemap[color];
    return QColor(r, g, b);
  }
  return QColor("red");
}

void draw_line(int x1,int y1,int x2,int y2,int color, void *painter) {
  PaintStruct *ps = (PaintStruct*)painter;
  x1 += ps->border;
  x2 += ps->border;
  y1 += ps->border;
  y2 += ps->border;
  ps->p->setPen(vcg2bcc(color));
  ps->p->drawLine(x1, y1, x2, y2);
}

QBrush b;

//---------------------------------------------------------------------------
void draw_rect(long x,long y,int w,int h,int color, void *painter) {
  PaintStruct *ps = (PaintStruct*)painter;
  x += ps->border;
  y += ps->border;
  ps->p->fillRect(x, y, w, h, vcg2bcc(color));
}

//---------------------------------------------------------------------------
void draw_poly(Point *hp, int n, int color, void *painter) {
  PaintStruct *ps = (PaintStruct*)painter;
  QPoint *pts = new QPoint[n];
  for ( int i=0; i < n; i++ ) {
    hp[i].x += ps->border;
    pts[i].setX(hp[i].x);
    hp[i].y += ps->border;
    pts[i].setY(hp[i].y);
  }
  QColor qc(vcg2bcc(color));
  ps->p->setPen(qc);
  QBrush br(qc);
  ps->p->setBrush(br);
  ps->p->drawPolygon(pts, n);
  delete[] pts;
}

//---------------------------------------------------------------------------
/* char *qtmpnam(char *answer) {
  // returns temporary file name
  // (abs path, includes directory, uses TEMP/TMP vars) {
  static int n = -1;

  const char *dir = getenv("TEMP");
  if (dir == NULL) {
    dir = getenv("TMP");
    if (dir == NULL) dir = "C:\\TEMP";
  }

  if (n == -1) n = time(NULL) * 1000;

  char *ptr = strcpy(answer, dir);
  ptr += strlen(ptr);
  if (ptr[-1] != '\\' && ptr[-1] != '/') {
    *ptr++ = '\\';
  }
  strcpy(ptr, "vcg12345.tmp");

  char *change = strchr(answer,0) - 9;
  while (1) {
    char buf2[20];
    _snprintf(buf2, sizeof(buf2), "%05u", n);
    int len = strlen(buf2);
    char *ptr = buf2 + len - 5;
    change[0] = ptr[0];
    change[1] = ptr[1];
    change[2] = ptr[2];
    change[3] = ptr[3];
    change[4] = ptr[4];
    if (access(answer, 0)) break;
    n++;
  }
  return answer;
} */

//---------------------------------------------------------------------------
/*
static char stderr_file[MAX_PATH]; long stderr_pos;
static char stdout_file[MAX_PATH]; long stdout_pos;

long display_stream(FILE *fp, long pos) {
  long np = ftell(fp);
  int len = np - pos;
  if ( len > 0 )
  {
    fseek(fp, pos, SEEK_SET);
    char *buf = new char[len+1];
    fread(buf, len, 1, fp);
    buf[len] = '\0';
    Form1->RichEdit1->Text = Form1->RichEdit1->Text + buf;
    Form1->RichEdit1->SelStart = Form1->RichEdit1->Text.Length() - 2;
    SendMessage(Form1->RichEdit1->Handle, EM_SCROLLCARET, 0, 0);
    delete buf;
    if ( Form1->Panel1->Height == 0 ) Form1->Panel1->Height = 100;
  }
  return np;
}

void display_streams(void) {
  stdout_pos = display_stream(stdout, stdout_pos);
  stderr_pos = display_stream(stderr, stderr_pos);
  Form1->Update();
}

void cleanup_streams(void) {
  fclose(stderr); unlink(stderr_file);
  fclose(stdout); unlink(stdout_file);
}
*/

void MainWindow::setNodesValue(const char *_nodes) {
  nodes->setText(_nodes);
}

//---------------------------------------------------------------------------
void MainWindow::drawGraph(PaintStruct *p) {
  draw_graph(draw_line, draw_rect, draw_poly, p);
}

//---------------------------------------------------------------------------
void error(const char *format, ...) {
  char errbuf[1024];
  va_list va;
  va_start(va, format);
  vsnprintf(errbuf, sizeof(errbuf), format, va);
  va_end(va);
  QMessageBox::critical(NULL, "Error", errbuf);
  exit(-1);
}



//---------------------------------------------------------------------------
const int STEP = 20;
const int PAGE = 100;

void MainWindow::KeyDown(int & Key, Qt::KeyboardModifiers Modifiers)
{
  switch ( Key )
  {
    case Qt::Key_Left:
      if ( (Modifiers & Qt::ControlModifier) != 0 )
        move_focus(-PAGE, 0);
      else
        move_focus(-STEP, 0);
      break;
    case Qt::Key_Right:
      if ( (Modifiers & Qt::ControlModifier) != 0 )
        move_focus(PAGE, 0);
      else
        move_focus(STEP, 0);
      break;
    case Qt::Key_Up:
      if ( (Modifiers & Qt::ControlModifier) != 0 )
        move_focus(0, -PAGE);
      else
        move_focus(0, -STEP);
      break;
    case Qt::Key_Down:
      if ( (Modifiers & Qt::ControlModifier) != 0 )
        move_focus(0, PAGE);
      else
        move_focus(0, STEP);
      break;
    /*case VK_PRIOR:
      move_focus(0, -ClientHeight / 2);
      break;
    case VK_NEXT:
      move_focus(0, ClientHeight / 2);
      break;*/
    default:
      return;
  }
  Key = 0;
}


//---------------------------------------------------------------------------
static int calc_scroll(int ScrollCode, int &ScrollPos, bool & reset_slider)
{
  static int old = 50;
  int code = 0;
  reset_slider = true;
  switch ( ScrollCode )
  {
  case QAbstractSlider::SliderSingleStepSub:	// User clicked the top or left scroll arrow or pressed the Up or Left arrow key.
    code = -STEP;
    break;
  case QAbstractSlider::SliderSingleStepAdd:	// User clicked the bottom or right scroll arrow or pressed the Down or Right arrow key.
    code = STEP;
    break;
  case QAbstractSlider::SliderPageStepSub:	// User clicked the area to the left of the thumb tab or pressed the PgUp key.
    code = -PAGE;
    break;
  case QAbstractSlider::SliderPageStepAdd:	// User clicked the area to the right of the thumb tab or pressed the PgDn key.
    code = PAGE;
    break;
  case QAbstractSlider::SliderMove:	// User is moving the thumb tab.  (5)
    code = -20*(old-ScrollPos);
    old = ScrollPos;
    reset_slider = false;
    break;
  case QAbstractSlider::SliderMove + 100://case scEndScroll:	// User finished moving the thumb tab on the scroll bar. (8)
    old = 50;
    ScrollPos = 50;
    break;
  case QAbstractSlider::SliderToMinimum:   	// User moved the thumb tab to the top or far left on the scroll bar.
  case QAbstractSlider::SliderToMaximum:	// User moved the thumb tab to the bottom or far right on the scroll bar.
    //case scPosition:	// User positioned the thumb tab and released it. (4)
    break;
  }
  ScrollPos = 50;
  return code;
}

//---------------------------------------------------------------------------
void MainWindow::scrollAction(int action)
{
  if ( action == QAbstractSlider::SliderNoAction )
    return;
  QAbstractSlider *slider = (QAbstractSlider *) QObject::sender();
  int ScrollPos = slider->sliderPosition();
  bool reset_slider;
  int code = calc_scroll(action, ScrollPos, reset_slider);
  if ( code != 0 )
  {
    if ( reset_slider )
      slider->setSliderPosition(ScrollPos);
    if ( slider == (QAbstractSlider *) canvas->verticalScrollBar() )
      move_focus(0, code);
    else
      move_focus(code, 0);
    canvas->viewport()->update();
  }
}

//---------------------------------------------------------------------------
void MainWindow::scrollSliderReleased()
{
  QAbstractSlider *slider = (QAbstractSlider *) QObject::sender();
  int ScrollPos = slider->sliderPosition();
  bool reset_slider;
  calc_scroll(QAbstractSlider::SliderMove + 100, ScrollPos, reset_slider);
  slider->setSliderPosition(ScrollPos);
}

//---------------------------------------------------------------------------
/*void MainWindow::vbarScroll(TObject *Sender, TScrollCode ScrollCode,
      int &ScrollPos) {
  int code = calc_scroll(ScrollCode, ScrollPos);
  if ( code != 0 )
  {
    move_focus(0, code);
    canvas->viewport()->update();
  }
}

//---------------------------------------------------------------------------
void MainWindow::hbarScroll(TObject *Sender, TScrollCode ScrollCode,
      int &ScrollPos) {
  int code = calc_scroll(ScrollCode, ScrollPos);
  if ( code != 0 )
  {
    move_focus(code, 0);
    canvas->viewport()->update();
  }
}

//---------------------------------------------------------------------------
void MainWindow::FormMouseWheel(TObject *Sender, TShiftState Shift,
      int WheelDelta, TPoint &MousePos, bool &Handled) {
  if ( Shift.Contains(ssShift) )
    move_focus(-WheelDelta, 0);
  else
    move_focus(0, -WheelDelta);
  Handled = true;
  canvas->viewport()->update();
}
*/

/*
//---------------------------------------------------------------------------
#define CUSTOM_ADDITIONAL_HEIGHT 36
#define CUSTOM_COMBO_X 1234
#define CUSTOM_COMBO_Y 1235

//---------------------------------------------------------------------------
static int AddList(HWND parent, int x, int y, int id) {
  int width = 40;
  int style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS;
  HWND combo = CreateWindow("COMBOBOX", "1", style, x, y, width, 23*10, parent,
                    HMENU(id), Application->Handle, NULL);
  SendMessage(combo, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), 0);
  char buf[2];
  for ( int i=1; i < 10; i++ )
  {
    buf[0] = '0' + i;
    buf[1] = '\0';
    SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)buf);
  }
  SendMessage(combo, CB_SETCURSEL, 0, 0);
  return width;
}

static int AddText(HWND parent, int x, int y, const char *text) {
  int width = 10;
  int style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | SS_LEFT;
  HWND txth = CreateWindow("STATIC", text, style, x, y, width, 23, parent,
                    0, Application->Handle, NULL);

  SendMessage(txth, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), 0);

  // calculate the text width
  HDC hdc = GetDC(txth);
  SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
  SIZE s;
  GetTextExtentPoint32(hdc, text, strlen(text), &s);
  ReleaseDC(txth, hdc);
  width = s.cx;

  // change the text control width
  SetWindowPos(txth, 0, 0, 0, width, 23, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
  return width;
}


BOOL CALLBACK enumfunc(HWND h, LPARAM lParam) {
  RECT *p = (RECT *)lParam;
  RECT rect;
  GetWindowRect(h, &rect);
  if ( rect.top >= p->bottom-CUSTOM_ADDITIONAL_HEIGHT-36 )
  {
    rect.top  += CUSTOM_ADDITIONAL_HEIGHT;
    POINT pnt;
    pnt.x = rect.left;
    pnt.y = rect.top;
    ScreenToClient(GetParent(h), &pnt);
    SetWindowPos(h, 0, pnt.x, pnt.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
  }
  return TRUE;
}

void MainWindow::PrinterSetupDialog1Show(TObject *Sender) {
  HWND h = PrinterSetupDialog1->Handle;
  SetWindowText(h, "Print");
  RECT rect;
  GetWindowRect(h, &rect);
  int cx = rect.right - rect.left;
  int cy = rect.bottom - rect.top;
  cy += CUSTOM_ADDITIONAL_HEIGHT;
  BOOL ok = SetWindowPos(h, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE);
  EnumChildWindows(h, WNDENUMPROC(enumfunc), LPARAM(&rect));  // move ok, cancel, network down

  ::GetClientRect(h, &rect);
  int x = 15;
  int y = rect.bottom - CUSTOM_ADDITIONAL_HEIGHT - 36;
    x +=   4 + AddText(h, x, y, "Print graph on");
    x +=   4 + AddList(h, x, y, CUSTOM_COMBO_X);
    x +=   4 + AddText(h, x, y, "by");
    x +=   4 + AddList(h, x, y, CUSTOM_COMBO_Y);
    x +=   4 + AddText(h, x, y, "pages");
}

//---------------------------------------------------------------------------
void MainWindow::PrinterSetupDialog1Close(TObject *Sender) {
  HWND h = PrinterSetupDialog1->Handle;
  xpages = SendDlgItemMessage(h, CUSTOM_COMBO_X, CB_GETCURSEL, 0, 0) + 1;
  ypages = SendDlgItemMessage(h, CUSTOM_COMBO_Y, CB_GETCURSEL, 0, 0) + 1;
}
*/

//---------------------------------------------------------------------------
static char *contents = NULL;
static long fsize;

void save_input_file_contents(FILE *fp) {
  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  if (fsize > 0) {
    contents = (char *)malloc(fsize);
    if (contents == NULL) {
      error("Not enough memory for the input file");
    }
    fseek(fp, 0, SEEK_SET);
    fsize = fread(contents, 1, fsize, fp);
  }
  fseek(fp, 0, SEEK_SET);
}

//---------------------------------------------------------------------------
// Added by AiO to open new gdl file
bool MainWindow::open() {
	
	QString fileName = QFileDialog::getOpenFileName(this,
        tr("Graph File"), "",
        tr("Graph Files (*.gdl);;All Files (*)"));
		
	// check filename type may need conversion
	
	if (fileName.isEmpty())
        return false;
	return loadFile(fileName);
}

bool MainWindow::saveAs() {
  QString fileName = QFileDialog::getSaveFileName(this);
  if (fileName.isEmpty()) {
    return false;
  }
  return saveFile(fileName);
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About WinGraph32"),
                     tr("<center>WinGraph32<br><br>"
                        "Copyright (c) 2011 Hex-Rays<br>"
                        "Version 1.10<br><br>"
                        "This program is based on VCG library written by<br>"
                        "Georg Sander and Iris Lemke<br><br>"
                        "WinGraph32 is released under GPL<br>"
                        "(GNU General Public License)<br><br>"
                        "<a href=\"http://www.hex-rays.com\">http://www.hex-rays.com</a>"
                        "</center>"));
}

void MainWindow::createActions() {
  printAct = new QAction(QIcon(":/images/print.png"), tr("&Print..."), this);
  //   printAct->setShortcut(QKeySequence::Print);
  printAct->setStatusTip(tr("Print the document"));
  connect(printAct, SIGNAL(triggered()), this, SLOT(printFile()));
  
  // Added by AiO to Open a new file
  openAct = new QAction(tr("&Open"), this);
  openAct->setShortcut(Qt::Key_O | Qt::CTRL);
  openAct->setStatusTip(tr("Open a new file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
  
  saveAsAct = new QAction(tr("Save &As..."), this);
  //   saveAsAct->setShortcut(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcut(Qt::Key_Escape);
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  normalAct = new QAction(QIcon(":/images/normalView.png"), tr("Normal view"), this);
  normalAct->setCheckable(true);
  normalAct->setChecked(true);
  normalAct->setShortcut(Qt::Key_N);
  normalAct->setStatusTip(tr("Normal view"));
  connect(normalAct, SIGNAL(triggered()), this, SLOT(normal()));

  polarAct = new QAction(QIcon(":/images/polar.png"), tr("Polar fish eye"), this);
  polarAct->setCheckable(true);
  polarAct->setShortcut(Qt::Key_P | Qt::CTRL);
  polarAct->setStatusTip(tr("Polar fish eye"));
  connect(polarAct, SIGNAL(triggered()), this, SLOT(polar()));

  polarFixedAct = new QAction(QIcon(":/images/fixedPolar.png"), tr("Fixed radius polar fish eye"), this);
  polarFixedAct->setCheckable(true);
  polarFixedAct->setShortcut(Qt::Key_P);
  polarFixedAct->setStatusTip(tr("Fixed radius polar fish eye"));
  connect(polarFixedAct, SIGNAL(triggered()), this, SLOT(polarFixed()));

  cartAct = new QAction(QIcon(":/images/cartesian.png"), tr("Cartesian fish eye"), this);
  cartAct->setCheckable(true);
  cartAct->setShortcut(Qt::Key_C | Qt::CTRL);
  cartAct->setStatusTip(tr("Cartesian fish eye"));
  connect(cartAct, SIGNAL(triggered()), this, SLOT(cart()));

  cartFixedAct = new QAction(QIcon(":/images/fixedCartesian.png"), tr("Fixed radius cartesian fish eye"), this);
  cartFixedAct->setCheckable(true);
  cartFixedAct->setShortcut(Qt::Key_C);
  cartFixedAct->setStatusTip(tr("Fixed radius cartesian fish eye"));
  connect(cartFixedAct, SIGNAL(triggered()), this, SLOT(cartFixed()));

  showLabelsAct = new QAction(QIcon(":/images/labelEdges.png"), tr("Display edge labels"), this);
  showLabelsAct->setCheckable(true);
  showLabelsAct->setStatusTip(tr("Display edge labels"));
  connect(showLabelsAct, SIGNAL(triggered()), this, SLOT(showLabels()));
  //   connect(showLabelsAct, SIGNAL(toggled(bool)), this, SLOT(toggleShowLabels(bool)));

  zoomInAct = new QAction(QIcon(":/images/zoomIn.png"), tr("Zoom in"), this);
  zoomInAct->setShortcut(Qt::Key_Plus);
  zoomInAct->setStatusTip(tr("Zoom in"));
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAct = new QAction(QIcon(":/images/zoomOut.png"), tr("Zoom out"), this);
  zoomOutAct->setShortcut(Qt::Key_Minus);
  zoomOutAct->setStatusTip(tr("Zoom out"));
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

  zoomNormalAct = new QAction(QIcon(":/images/zoomNormal.png"), tr("Zoom normal"), this);
  zoomNormalAct->setShortcut(Qt::Key_0);
  zoomNormalAct->setStatusTip(tr("Zoom normal (100%)"));
  connect(zoomNormalAct, SIGNAL(triggered()), this, SLOT(zoomNormal()));

  fitAllAct = new QAction(QIcon(":/images/fitAll.png"), tr("Fit all graph"), this);
  fitAllAct->setShortcut(Qt::Key_M);
  fitAllAct->setStatusTip(tr("Fit all graph"));
  connect(fitAllAct, SIGNAL(triggered()), this, SLOT(fitAll()));

  positOriginAct = new QAction(QIcon(":/images/positOrigin.png"), tr("Position on origin"), this);
  positOriginAct->setShortcut(Qt::Key_O);
  positOriginAct->setStatusTip(tr("Position on origin"));
  connect(positOriginAct, SIGNAL(triggered()), this, SLOT(positOrigin()));

  easyPanAct = new QAction(tr("Easy graph panning"), this);
  easyPanAct->setCheckable(true);
  easyPanAct->setStatusTip(tr("Easy graph panning"));
  connect(easyPanAct, SIGNAL(triggered()), this, SLOT(easyPan()));

  helpAct = new QAction(tr("Help"), this);
  helpAct->setShortcut(Qt::Key_F1);
  helpAct->setStatusTip(tr("Display help"));
  connect(helpAct, SIGNAL(triggered()), this /*qApp*/, SLOT(help()));

  aboutAct = new QAction(tr("&About..."), this);
  aboutAct->setStatusTip(tr("Show the About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
  
  //Added by AiO  new action for search window
  searchAct = new QAction(tr("&Search"), this);
  searchAct->setShortcut(Qt::Key_F5);
  searchAct->setStatusTip(tr("Search F5"));
  connect(searchAct, SIGNAL(triggered()), this, SLOT(showSearch()));

}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);			//Added by AiO
  fileMenu->addAction(printAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(normalAct);
  viewMenu->addAction(polarAct);
  viewMenu->addAction(polarFixedAct);
  viewMenu->addAction(cartAct);
  viewMenu->addAction(cartFixedAct);
  viewMenu->addSeparator();
  viewMenu->addAction(showLabelsAct);

  zoomMenu = menuBar()->addMenu(tr("&Zoom"));
  zoomMenu->addAction(zoomInAct);
  zoomMenu->addAction(zoomOutAct);
  zoomMenu->addAction(zoomNormalAct);
  zoomMenu->addAction(fitAllAct);

  moveMenu = menuBar()->addMenu(tr("&Move"));
  moveMenu->addAction(positOriginAct);
  moveMenu->addAction(easyPanAct);
  moveMenu->addAction(searchAct);			//Added by AiO

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(helpAct);
  helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars() {
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->setIconSize(QSize(20, 19));
  fileToolBar->addAction(printAct);

  zoomToolBar = addToolBar(tr("Zoom"));
  zoomToolBar->setIconSize(QSize(20, 19));
  zoomToolBar->addAction(zoomInAct);
  zoomToolBar->addAction(zoomOutAct);
  zoomToolBar->addAction(fitAllAct);
  zoomToolBar->addAction(zoomNormalAct);
  zoomToolBar->addAction(positOriginAct);

  viewToolBar = addToolBar(tr("View"));
  viewToolBar->setIconSize(QSize(20, 19));
  viewToolBar->addAction(normalAct);
  viewToolBar->addAction(polarAct);
  viewToolBar->addAction(polarFixedAct);
  viewToolBar->addAction(cartAct);
  viewToolBar->addAction(cartFixedAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(showLabelsAct);

}

void MainWindow::createStatusBar() {
  zoom = new QLabel("100.00%");
  origin = new QLabel("0, 0");
  nodes = new QLabel("0");
  myStatusBar = new QStatusBar();
  myStatusBar->insertWidget(0, zoom);
  myStatusBar->insertWidget(1, origin);
  myStatusBar->insertWidget(2, nodes);
  setStatusBar(myStatusBar);
}

void MainWindow::readSettings() {
  QSettings settings("Hex-Rays", "WinGraph32");
  G_displayel = settings.value("EdgeLabels", true).toBool();
  showLabelsAct->setChecked(G_displayel);
  setShowLabelsIcon();
  sticky = settings.value("StickMouse", false).toBool();
  easyPanAct->setChecked(sticky);
  QPoint pos = settings.value("pos", QPoint(360, 135)).toPoint();
  QSize size = settings.value("size", QSize(542, 485)).toSize();
  resize(size);
  move(pos);
}

void MainWindow::writeSettings() {
  QSettings settings("Hex-Rays", "WinGraph32");
  settings.setValue("pos", pos());
  settings.setValue("size", size());
  settings.setValue("EdgeLabels", G_displayel);
  settings.setValue("StickMouse", sticky);
}

bool MainWindow::loadFile(const QString &fileName) {
	char buf[1024];
	char localFileName[800];
	QByteArray pstr;
	
	// convert fileName to localFileName
	// copy localFileName to Dataname
	pstr = fileName.toLatin1();						//possible alt .toAscii().data()
	strncpy(localFileName, pstr.data(), 800-1);
	strncpy(Dataname, localFileName, 800-1);
	parse_part();
	visualize_part();
	display_complete_graph();
	update_zoom_status();
	canvas->viewport()->update();
	_snprintf(buf, sizeof(buf), "WinGraph32 - %s", localFileName);
	setWindowTitle(buf);
	statistics();
	_snprintf(buf, sizeof(buf), "%d nodes, %d edge segments, %d crossings",
            st_nr_vis_nodes,
            st_nr_vis_edges,
            nr_crossings);
	setNodesValue(buf);

	resize(800, 600);
	return true;
}

bool MainWindow::saveFile(const QString &fileName) {
  FILE *fp = fopen(fileName.toAscii().data(), "w");
  if (fp == NULL) {
    QMessageBox::warning(this, tr("Application"),
                         tr("Cannot open file %1.\n")
                         .arg(fileName));
    return false;
  }

  if (fwrite(contents, 1, fsize, fp) != fsize) {
    QMessageBox::warning(this, tr("Application"),
                         tr("Cannot write to file %1.\n")
                         .arg(fileName));
  }
  fclose(fp);
  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File saved"), 2000);
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName) {
  curFile = fileName;

  QString shownName = curFile;
  if (curFile.isEmpty()) {
    shownName = "untitled.txt";
  }
  setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName) {
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::printFile()
{
  GRPrintPagesDlg pagesdlg;
  if ( pagesdlg.exec() != QDialog::Accepted )
    return;

  int rows = pagesdlg.rows();
  int columns = pagesdlg.columns();

  QPrintDialog pr;
  if ( pr.exec() == QDialog::Accepted )
  {
    QPrinter *qp = pr.printer();

    int pwidth = qp->pageRect().width();
    int pheight = qp->pageRect().height();

    int npages = rows * columns;

    int gwidth = columns * pwidth, gheight = rows * pheight;

    QImage img(gwidth, gheight, QImage::Format_RGB444);

    if ( img.isNull() )
      return;

    // render graph

    QPainter gpaint;
    gpaint.begin(&img);

    gpaint.fillRect(img.rect(), Qt::white);

    QFont f("courier", 12);
    gpaint.setFont(f);

    QRect text_rect = gpaint.fontMetrics().boundingRect(G_title);
    int x = (gwidth - text_rect.width()) / 2;
    gpaint.drawText(x, text_rect.height(), G_title);

    PaintStruct ps;
    ps.p = &gpaint;
    ps.border = 60;
    set_drawing_rectangle(gwidth-120, gheight-120);
    display_complete_graph();
    draw_graph(draw_line, draw_rect, draw_poly, &ps);

    gpaint.end();

    // copy to pages

    QPainter prpaint;
    prpaint.begin(qp);

    QRect src;
    QRect dst(0, 0, pwidth, pheight);
    for ( int page = 0, col = 0, row = 0; page < npages; ++page )
    {
      src.setLeft(col*pwidth);
      src.setTop(row*pheight);
      src.setWidth(pwidth);
      src.setHeight(pheight);
      prpaint.drawImage(QPoint(0, 0), img, src);

      // next
      if ( page < npages - 1 )
        qp->newPage();

      if ( col < columns - 1 )
        ++col;
      else
      {
        ++row;
        col = 0;
      }
    }

    prpaint.end();

    //this will reset the drawing rectangle to the canvas values
    canvas->viewport()->repaint();
    display_complete_graph();
  }
}
// Added by AiO
void MainWindow::showSearch()
{
	Search_Dialog sd;
	sd.exec();
	return;
}
void MainWindow::normal() {
  //   normalAct->setIcon(QIcon(":/images/normalViewDown.png"));
  polarAct->setChecked(false);
  polarFixedAct->setChecked(false);
  cartAct->setChecked(false);
  cartFixedAct->setChecked(false);
  normalAct->setChecked(true);
  m_validate_fe(4);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::polar() {
  polarAct->setChecked(true);
  polarFixedAct->setChecked(false);
  cartAct->setChecked(false);
  cartFixedAct->setChecked(false);
  normalAct->setChecked(false);
  m_validate_fe(0);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::polarFixed() {
  polarAct->setChecked(false);
  polarFixedAct->setChecked(true);
  cartAct->setChecked(false);
  cartFixedAct->setChecked(false);
  normalAct->setChecked(false);
  m_validate_fe(1);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::cart() {
  polarAct->setChecked(false);
  polarFixedAct->setChecked(false);
  cartAct->setChecked(true);
  cartFixedAct->setChecked(false);
  normalAct->setChecked(false);
  m_validate_fe(2);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::cartFixed() {
  polarAct->setChecked(false);
  polarFixedAct->setChecked(false);
  cartAct->setChecked(false);
  cartFixedAct->setChecked(true);
  normalAct->setChecked(false);
  m_validate_fe(3);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::setShowLabelsIcon() {
  if (showLabelsAct->isChecked()) {
    showLabelsAct->setIcon(QIcon(":/images/labelEdgesDown.png"));
  }
  else {
    showLabelsAct->setIcon(QIcon(":/images/labelEdges.png"));
  }
}

void MainWindow::showLabels() {
  G_displayel = showLabelsAct->isChecked();
  if (G_displayel) {
    showLabelsAct->setIcon(QIcon(":/images/labelEdgesDown.png"));
  }
  else {
    showLabelsAct->setIcon(QIcon(":/images/labelEdges.png"));
  }
  relayout();
  canvas->viewport()->update();
}

void MainWindow::zoomIn() {
  change_fe_scaling(3, 2);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::zoomOut() {
  change_fe_scaling(2, 3);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::zoomNormal() {
  set_fe_scaling(1, 1);
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::fitAll() {
  display_complete_graph();
  update_zoom_status();
  canvas->viewport()->update();
}

void MainWindow::positOrigin() {
  normal_fe_focus();
  canvas->viewport()->update();
  update_origin_status();
}

void MainWindow::easyPan() {
  sticky = !sticky;
  if ( sticky )
  {
    //enable easy pan mode
  }
  else {
    //disable easy pan mode
    qApp->restoreOverrideCursor();
  }
}

void MainWindow::help() {
  QMessageBox::information(this, tr("WinGraph32 Help"),
                           tr(
                               "The purpose of WinGraph32 is to visualize graphs which consist of nodes and edges.<br><br>"

                               "WinGraph32 tool reads a textual and readable specification of a graph and "
                               "visualizes  the graph. It layouts the graph using several heuristics as reducing "
                               "the number of crossings,  minimizing the  size of  edges,  centering of nodes. "
                               "The specification language of WinGraph32 is GDL, Graph Description "
                               "Language.<br><br>"

                               "The user interface:<ul>"

                               "N - normal view<br>"
                               "P - polar fish view with fixed radius<br>"
                               "Ctrl-P - polar fish view<br>"
                               "C - cartesian fish view with fixed radius<br>"
                               "Ctrl-C - cartesian fish view<br><br>"

                               "Numpad + - zoom in 150%<br>"
                               "Numpad - - zoom out 66%<br>"
                               "M - fit the whole graph<br>"
                               "0 - normal (100%) zoom<br><br>"

                               "O - position on origin<br>"
                               "MouseWheel - scroll vertically<br>"
                               "Shift-MouseWheel - scroll horizontally<br>"
                               "Arrows - scroll, Ctrl-arrows - scroll faster<br>"
                               "Shift-LeftClick - zoomin, Ctrl-LeftCLick - zoomout"
                               "</ul>"

                               "You can pan the graph by dragging it in any direction. If the \"easy graph "
                               "panning\" is checked then the drag operation is started after any mouse click. "
                               "In this case you may release the mouse button and the panning will continue."));
}

bool MainWindow::close() {
  writeSettings();
  return QMainWindow::close();
}



bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
   
  QClipboard * clipboard = QApplication::clipboard();		//added by AiO 
  if ( obj == canvas->viewport() )
  {
    // paint
    if ( event->type() == QEvent::Paint )
    {
      set_drawing_rectangle(canvas->viewport()->size().width(), canvas->viewport()->size().height());
      QPainter p(canvas->viewport());
      p.setPen(Qt::black);
      p.setBrush(Qt::green);
      PaintStruct ps;
      ps.p = &p;
      ps.border = 2;
      drawGraph(&ps);
    }

    // mouse
    else if ( event->type() == QEvent::MouseMove )
    {
      QMouseEvent *mevent = (QMouseEvent *) event;
      if ( qApp->overrideCursor() )
      {
        //cursor is overridden, we are panning
        if ( mevent->x() != ox || mevent->y() != oy )
        {
          move_focus(ox - mevent->x(), oy - mevent->y());
          ox = mevent->x();
          oy = mevent->y();
          canvas->viewport()->update();
          //update();
        }
      }
    }
    else if ( event->type() == QEvent::MouseButtonPress )
    {
      QMouseEvent *mevent = (QMouseEvent *) event;
      if ( mevent->button() == Qt::LeftButton )
      {
        Qt::KeyboardModifiers mods = mevent->modifiers();
        if (mods & Qt::ShiftModifier) zoomIn();
        else if (mods & Qt::ControlModifier) zoomOut();
        if ( !sticky )
        {
          qApp->setOverrideCursor(*MOVING_CURSOR);
        }
        else
        {
          if ( qApp->overrideCursor() )
          {
            //cursor is already overridden, release it
            qApp->restoreOverrideCursor();
            canvas->setMouseTracking(false);
          }
          else
          {
            qApp->setOverrideCursor(*MOVING_CURSOR);
            canvas->setMouseTracking(true);
          }
        }
        ox = mevent->x();
        oy = mevent->y();
      }
	  else if(mevent->button() == Qt::RightButton )		//Added by AiO
	  {
		  //query the cordinates of the mouse for node
		  char curNodeTitle[1024];
		  char * curNode;
		  char *index;	//used for search
		  QMessageBox msgBox;
		  curNode = node_xy_title(mevent->x(),mevent->y());
		  if(curNode == 0) return true;
		  strncpy(curNodeTitle, curNode, 1024-1);
		  curNodeTitle[1024-1] = 0;
		  //label ends with ':'
		  //test for color characters format sym num num TEXT sym :
		  
		  // replace : -1 with 0 and use &curNodeTitle[3]
		  
		  
		  //test with popup
		  if(curNodeTitle[0] == 0x0c)
		  {
			index = strchr(curNodeTitle, ':');
			if(index == 0) goto nocolon; //test for no label but color
			--index;
			--index;
			--index;
			*index = 0;
			msgBox.setText(&curNodeTitle[3]);
			clipboard->setText(&curNodeTitle[3]);
		  }
		  else
		  {
			  nocolon:
			msgBox.setText(curNodeTitle);
		    clipboard->setText(curNodeTitle);
		  }
		  msgBox.exec();
	  }
    }
    else if ( event->type() == QEvent::MouseButtonRelease )
    {
      if ( !sticky )
        qApp->restoreOverrideCursor();
    }

    // wheel
    else if ( event->type() == QEvent::Wheel )
    {
      QWheelEvent *wevent = (QWheelEvent *) event;
      if ( (qApp->keyboardModifiers() & Qt::ControlModifier) != 0 )
      {
        wevent->accept();
        int numDegrees = wevent->delta() / 8;
        int numSteps = numDegrees / 15;
        if ( numSteps < 0)
        {
          numSteps = -numSteps;
          for ( int x = 0; x < numSteps; x++ )
            zoomOut();
        }
        else
        {
          for ( int x = 0; x < numSteps; x++ )
            zoomIn();
        }
        return true;
      }
      else
      {
        bool ret = QMainWindow::eventFilter(obj, event);
        QAbstractSlider *slider = (QAbstractSlider *) (wevent->orientation() == Qt::Vertical?
                                                       canvas->verticalScrollBar() :
                                                       canvas->horizontalScrollBar());
        int ScrollPos = slider->sliderPosition();
        bool reset_slider;
        calc_scroll(QAbstractSlider::SliderMove + 100, ScrollPos, reset_slider);
        slider->setSliderPosition(ScrollPos);
        return ret;
      }
    }

    // keyboard
    else if ( event->type() == QEvent::KeyPress )
    {
      QKeyEvent *kevent = (QKeyEvent *) event;
      int key = kevent->key();
      KeyDown(key, kevent->modifiers());
      if ( key == 0 )
      {
        event->accept();
        return true;
      }
    }
  }

  return QMainWindow::eventFilter(obj, event);
}

//---------------------------------------------------------------------------

MainWindow::MainWindow() {
  char buf[1024];
  Form1 = this;

  fgColor = vcg2bcc(G_color);

  canvas = new QAbstractScrollArea();
  canvas->viewport()->setBackgroundRole(QPalette::Base);
  canvas->viewport()->setAutoFillBackground(true);
  canvas->viewport()->setFocusPolicy(Qt::StrongFocus);
  canvas->viewport()->installEventFilter(this);
  canvas->verticalScrollBar()->setRange(0, 100);
  canvas->verticalScrollBar()->setValue(50);
  canvas->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  canvas->horizontalScrollBar()->setRange(0, 100);
  canvas->horizontalScrollBar()->setValue(50);
  canvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  connect(canvas->verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(scrollAction(int)));
  connect(canvas->horizontalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(scrollAction(int)));
  connect(canvas->verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(scrollSliderReleased()));
  connect(canvas->horizontalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(scrollSliderReleased()));
  setCentralWidget(canvas);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();

  readSettings();

  MOVING_CURSOR = new QCursor(Qt::OpenHandCursor);

  setCurrentFile("");
  setUnifiedTitleAndToolBarOnMac(true);

  update_origin_status();
  set_drawing_rectangle(canvas->size().width() - 4, canvas->size().height() - 4);

  QStringList args = qApp->arguments();

  int argc = args.size();
  char **argv = new char*[argc+1];
  for (int i = 0; i < argc; i++ ) {
    argv[i] = _strdup(args.at(i).toAscii().data());
  }
  argv[argc] = NULL;
  vcg_main(argc, argv);
  display_complete_graph();
  update_zoom_status();
  _snprintf(buf, sizeof(buf), "WinGraph32 - %s", G_title);
  setWindowTitle(buf);
  statistics();
  _snprintf(buf, sizeof(buf), "%d nodes, %d edge segments, %d crossings",
            st_nr_vis_nodes,
            st_nr_vis_edges,
            nr_crossings);
  setNodesValue(buf);

  resize(800, 600);
}
