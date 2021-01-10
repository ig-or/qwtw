
//#include <QLayout>
#include <QtOpenGL>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QScrollArea>
#include <QTimer>

#include <mgl2/qmathgl.h>
#include "qmgl.h"

#include "xpm/alpha.xpm"
#include "xpm/arc.xpm"
#include "xpm/copy.xpm"
#include "xpm/curve.xpm"
#include "xpm/down_1.xpm"
#include "xpm/fileprint.xpm"
#include "xpm/left_1.xpm"
#include "xpm/light.xpm"
#include "xpm/line.xpm"
#include "xpm/mark_a.xpm"
#include "xpm/mark_d.xpm"
#include "xpm/mark_o.xpm"
#include "xpm/mark_s.xpm"
#include "xpm/right_1.xpm"
#include "xpm/next_sl.xpm"
#include "xpm/norm_1.xpm"
#include "xpm/ok.xpm"
#include "xpm/prev_sl.xpm"
#include "xpm/rotate.xpm"
#include "xpm/show_sl.xpm"
#include "xpm/text.xpm"
#include "xpm/polygon.xpm"
#include "xpm/zoom_1.xpm"
#include "xpm/zoom_in.xpm"
#include "xpm/zoom_out.xpm"
#include "xpm/up_1.xpm"
#include "xpm/stop.xpm"
#include "xpm/pause.xpm"

#include <float.h>

OurMathGL::OurMathGL(QWidget *parent, Qt::WindowFlags f) : QMathGL(parent, f) {

}

OurMathGL::~OurMathGL() {
	timer->stop();	timerRefr->stop();
	if(gr && mgl_use_graph(gr,-1)<1)	mgl_delete_graph(gr);
	//  private if(grBuf)	delete []grBuf;
	//  ???? if(draw)	delete draw;
	gr = 0;
	draw = 0;// !!!!
}



int sample(mglGraph* gr)
{
	const int N = 100;
	double x[N];
	double y[N];
	double z[N];
	double t;
	double R = 10.0;

	for (int i = 0; i < N; i++) {
		t = (double(i) / double(N)) * 2. * 3.14159 * 3.5;
		x[i] = R * sin(t);
		y[i] = R * cos(t);
		z[i] = t;
	}

	mglData mx(N, x);
	mglData my(N, y);
	mglData mz(N, z);

	//gr->SetTickRotate(false);
	//gr->SetAxisStl("k", 1, 2);
	gr->SubPlot(1, 1, 0);
	gr->Rotate(50,60);

	//gr->SetOrigin(0., 0., 0.);
	//gr->SetPlotFactor(0);
	gr->SetRanges(-R, R, -R, R, -0.0, 2.*3.14 * 3.5);
	gr->Axis("xyz AKDTVISO a 4 : E"); 
	gr->Grid();
	gr->Box();
	//gr->Adjust();


	//gr->Title("THE TITLE");
	
	//gr->Rotate(60,40);

	gr->Plot(mx, my, mz,"rs");
	gr->Label('x',"X",1);
	gr->Label('y',"Y",1);
	gr->Label('z',"Z",1);
	//gr->Box();
	
  	return 0;
}

void QMGL1::addLine(int size, double* x, double* y, double* z) {
	mx = mglData(size, x);
	my = mglData(size, y);
	mz = mglData(size, z);

	double xMin1 = DBL_MAX, xMax1 = -DBL_MAX;
	double yMin1 = DBL_MAX, yMax1 = -DBL_MAX, zMin1 = DBL_MAX, zMax1 = -DBL_MAX;
	for (int i = 0; i < size; i++) {
		if (x[i] > xMax1) { xMax1 = x[i]; }
		if (x[i] < xMin1) { xMin1 = x[i]; }

		if (y[i] > yMax1) { yMax1 = y[i]; }
		if (y[i] < yMin1) { yMin1 = y[i]; }

		if (z[i] > zMax1) { zMax1 = z[i]; }
		if (z[i] < zMin1) { zMin1 = z[i]; }
	}

	if (linesCount == 0) {
		//gr->SubPlot(1, 1, 0);
		//gr->Rotate(50,60);
		//gr->SetOrigin(0., 0., 0.);
		xMin = xMin1; xMax = xMax1; 
		yMin = yMin1; yMax = yMax1; 
		zMin = zMin1; zMax = zMax1; 
		
		//gr->SetRanges(xMin1, xMax1, yMin1, yMax1, zMin1, zMax1);

		//gr->Axis("xyz AKDTVISO a 4 : E"); 

		//gr->Grid();
		//gr->Box();

		//gr->Adjust();
		//gr->Title("THE TITLE");
	} else {
		bool rangeChanged = false;
		if (xMax1 > xMax) { rangeChanged = true; }
		if (xMin1 > xMin) { rangeChanged = true; }

		if (yMax1 > yMax) { rangeChanged = true; }
		if (yMin1 > yMin) { rangeChanged = true; }

		if (zMax1 > zMax) { rangeChanged = true; }
		if (zMin1 > zMin) { rangeChanged = true; }

		if (rangeChanged) {		
			xMin = xMin1; xMax = xMax1; 
			yMin = yMin1; yMax = yMax1; 
			zMin = zMin1; zMax = zMax1; 
			
			//gr->SetRanges(xMin1, xMax1, yMin1, yMax1, zMin1, zMax1);
		}
		
	}

	//gr->Plot(mx, my, mz,"rs");
	//gr->Label('x',"X",1);
	//gr->Label('y',"Y",1);
	//gr->Label('z',"Z",1);
	linesCount += 1;


	mgl->update();
	
}


QMGL1::QMGL1(QWidget *parent) : QWidget(parent) {
	printf("creating QMGL1 widget start .. \n");
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(2); layout->setMargin(2);

	QFrame* top_frame = new QFrame(this);
	top_frame->setMinimumSize(QSize(0, 32));
	top_frame->setMaximumHeight(32);
	top_frame->setFrameShape(QFrame::NoFrame);
	//top_frame->setFrameShadow(QFrame::Raised);
	top_frame->setLineWidth(1);
	endOfResizeFlag = 0;

	tool_frame = new QFrame(this);
	tool_frame->setFrameShape(QFrame::NoFrame);
	tool_frame->setMinimumSize(QSize(0, 32));
	tool_frame->setMaximumHeight(64);
	tool_frame->setLineWidth(1);

	toolLayout = new QBoxLayout(QBoxLayout::LeftToRight, tool_frame);
	//set margins to zero so the toolbar touches the widget's edges
    toolLayout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* horizontalLayout = new QHBoxLayout(top_frame);
	horizontalLayout->setSpacing(2);
	horizontalLayout->setMargin(2);

	menu_bar = new QMenuBar();
	// mgl = new QMathGL(this);
	mgl = new OurMathGL(0);
	linesCount = 0;
	//mgl->setDraw(sample);

	//mgl->get
	//gr = (mglGraph*)mgl->getGraph();
	//gr = new mglGraph();
	
	mgl->setDraw(this);

	mgl->setZoom(true);
	mgl->setRotate(true);
	mgl->autoResize = true;
	drawCounter = 0;
	//mgl->set

	QSpacerItem* horizontalSpacer = new QSpacerItem(244, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer);

	QScrollArea* scroll = new QScrollArea(this);
	scroll->setWidget(mgl);
	//printf("adding menu ...\n");
	addMenu();
	//printf("menu added \n");

	layout->addWidget(menu_bar);
	layout->addWidget(tool_frame);
	//layout->addWidget(top_frame);
	layout->addWidget(scroll);
	setLayout(layout);

	//mgl->adjust();
	mgl->update();	
	resizeTimer = new QTimer(this);
	connect(resizeTimer, &QTimer::timeout, this, &QMGL1::endOfResize);

	QTimer::singleShot(250, this, &QMGL1::polish);
	//printf("creating QMGL1 widget end .. \n");
}

int QMGL1::Draw(mglGraph * gr) {
	if (endOfResizeFlag != 0) {
		endOfResizeFlag -= 1;
		return 0;
	}
	drawCounter += 1;
	printf("Draw. line Count = %d; drawCounter = %d \n", linesCount, drawCounter);

	if (linesCount == 0) {
		return 0;
	}
	//return 0;

	gr->SubPlot(1, 1, 0);
	gr->Rotate(50,60);
	//gr->SetOrigin(0., 0., 0.);
	
	gr->SetRanges(xMin, xMax, yMin, yMax, zMin, zMax);

	gr->Axis("xyz AKDTVISO a 4 : E"); 

	//gr->Grid();
	//gr->Box();

	gr->Adjust();

	gr->Plot(mx, my, mz,"rs");
	//printf("eod\n");
	return 0;
}

void QMGL1::polish() {

	mgl->setZoom(true);
	mgl->setRotate(true);
	mgl->autoResize = true;

	mgl->adjust();
	//mgl->update();
	//printf("polish\n");
}

void QMGL1::endOfResize() {
	printf("endOfResize start\n");
	endOfResizeFlag = 1;
	mgl->adjust();
	//mgl->update();
	printf("endOfResize stop\n");
	resizeTimer->stop();
}

void QMGL1::resizeEvent(QResizeEvent *event) {
	//printf("resizeEvent\n");
	if (resizeTimer->isActive()) {
		resizeTimer->stop();
	}
	resizeTimer->start(400);
}

QMGL1::~QMGL1() {

}

void QMGL1::ensurePolished() {
//	mgl->update();
	mgl->adjust();
	mgl->update();
	//printf("ensurePolished\n");
}

void QMGL1::addMenu() {
	
	QAction *a;
	QMenu *o, *oo, *f;
	QToolBar *bb;

	QMenu *popup = new QMenu(this);
	
	// file menu
	//{
		/*
		f = o = menu_bar->addMenu(("File"));
		oo = new QMenu(("Export as 2D ..."),this);
		oo->addAction(("PNG"), mgl, SLOT(exportPNG()),Qt::ALT+Qt::Key_P);
		oo->addAction(("solid PNG"), mgl, SLOT(exportPNGs()),Qt::ALT+Qt::Key_F);
		oo->addAction(("JPEG"), mgl, SLOT(exportJPG()),Qt::ALT+Qt::Key_J);
		oo->addAction(("bitmap EPS"), mgl, SLOT(exportBPS()));
		oo->addAction(("vector EPS"), mgl, SLOT(exportEPS()),Qt::ALT+Qt::Key_E);
		oo->addAction(("SVG"), mgl, SLOT(exportSVG()),Qt::ALT+Qt::Key_S);
		oo->addAction(("LaTeX"), mgl, SLOT(exportTEX()),Qt::ALT+Qt::Key_L);
		o->addMenu(oo);		popup->addMenu(oo);
		oo = new QMenu(_("Export as 3D ..."),this);
		oo->addAction(_("MGLD"), mgl, SLOT(exportMGLD()),Qt::ALT+Qt::Key_M);
		oo->addAction(_("PRC"), mgl, SLOT(exportPRC()),Qt::ALT+Qt::Key_D);
		oo->addAction(_("OBJ"), mgl, SLOT(exportOBJ()),Qt::ALT+Qt::Key_O);
		oo->addAction(_("STL"), mgl, SLOT(exportSTL()));
		oo->addAction(_("XYZ"), mgl, SLOT(exportXYZ()));
//		oo->addAction(_("X3D"), mgl, SLOT(exportX3D()),Qt::ALT+Qt::Key_X);
		o->addMenu(oo);		popup->addMenu(oo);

		o->addSeparator();
		a = new QAction(QPixmap(fileprint), _("Print graphics"), this);
		this->connect(a, SIGNAL(triggered()), mgl, SLOT(print()));
		a->setToolTip(_("Open printer dialog and print graphics (Ctrl+P)"));
		a->setShortcut(Qt::CTRL+Qt::Key_P);	o->addAction(a);
		o->addSeparator();
		o->addAction(_("Close"), this, SLOT(close()), Qt::CTRL+Qt::Key_W);
		*/
	//}
	// graphics menu

	
	printf("adding graphics menu \n");
	{
		bb = new QToolBar(("Graphics"),this);
		toolLayout->addWidget(bb, Qt::AlignLeft);

		o = menu_bar->addMenu(("Graphics"));

		printf(" 1 ");

		a = new QAction(QPixmap(alpha_xpm), ("Alpha"), this);
		a->setShortcut(Qt::ALT+Qt::Key_T);	a->setCheckable(true);
		connect(a, SIGNAL(toggled(bool)), mgl, SLOT(setAlpha(bool)));
		connect(mgl, SIGNAL(alphaChanged(bool)), a, SLOT(setChecked(bool)));
		a->setToolTip(("Switch on/off transparency for the graphics (Alt+T)."));
		o->addAction(a);		bb->addAction(a);

//printf(" 10 ");
		a = new QAction(QPixmap(light_xpm), ("Light"), this);
		a->setShortcut(Qt::ALT+Qt::Key_L);	a->setCheckable(true);
		connect(a, SIGNAL(toggled(bool)), mgl, SLOT(setLight(bool)));
		connect(mgl, SIGNAL(lightChanged(bool)), a, SLOT(setChecked(bool)));
		a->setToolTip(("Switch on/off lightning for the graphics (Alt+L)."));
		o->addAction(a);		bb->addAction(a);

//printf(" 11 ");
		a = new QAction(QPixmap(rotate_xpm), ("Rotate by mouse"), this);
		a->setCheckable(true);
		connect(a, SIGNAL(toggled(bool)), mgl, SLOT(setRotate(bool)));
		connect(mgl, SIGNAL(rotateChanged(bool)), a, SLOT(setChecked(bool)));
		a->setToolTip(("Switch on/off mouse handling of the graphics\n(rotation, shifting, zooming and perspective)."));
		bb->addAction(a);

//printf(" 12 ");
		a = new QAction(QPixmap(zoom_in_xpm), ("Zoom by mouse"), this);
		a->setCheckable(true);
		connect(a, SIGNAL(toggled(bool)), mgl, SLOT(setZoom(bool)));
		connect(mgl, SIGNAL(zoomChanged(bool)), a, SLOT(setChecked(bool)));
		a->setToolTip(("Switch on/off mouse zoom of selected region."));
		bb->addAction(a);
		o->addSeparator();

//printf(" 13 ");
		a = new QAction(QPixmap(zoom_out_xpm), ("Restore"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(restore()));
		a->setToolTip(("Restore default graphics rotation, zoom and perspective (Alt+Space)."));
		a->setShortcut(Qt::ALT+Qt::Key_Space);
		o->addAction(a);	bb->addAction(a);	popup->addAction(a);
		bb->addSeparator();

		o->addAction(a);	bb->addAction(a);	popup->addAction(a);

//printf(" 14 ");
		a = new QAction(QPixmap(ok_xpm), ("Redraw"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(update()));
		a->setToolTip(("redraw graphics (F5)."));
		a->setShortcut(Qt::Key_F5);
		o->addAction(a);	bb->addAction(a);	popup->addAction(a);

//printf(" 15 ");
		a = new QAction(QPixmap(stop_xpm), ("Stop"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(stop()));
		a->setToolTip(("Ask to stop plot drawing (F7)."));
		a->setShortcut(Qt::Key_F7);

//printf(" 16 ");
		a = new QAction(("Adjust size"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(adjust()));
		a->setToolTip(("Change canvas size to fill whole region (F6)."));
		a->setShortcut(Qt::Key_F6);		o->addAction(a);

//printf(" 17 ");
		a = new QAction(QPixmap(copy_xpm), ("Copy plot"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(copy()));
		a->setToolTip(("Copy graphics to clipboard (Ctrl+Shift+G)."));
		a->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_G);
		o->addAction(a);		bb->addAction(a);	popup->addAction(a);
		bb->addSeparator();

//printf(" 18 ");
		teta = new QSpinBox(tool_frame);	teta->setWrapping(true);
		bb->addWidget(teta);	teta->setRange(-180, 180);	teta->setSingleStep(10);
		connect(teta, SIGNAL(valueChanged(int)), mgl, SLOT(setTet(int)));
		connect(mgl, SIGNAL(tetChanged(int)), teta, SLOT(setValue(int)));
		teta->setToolTip(("Set value of \\theta angle."));
		bb->addSeparator();

//printf(" 19 ");
		phi = new QSpinBox(tool_frame);	phi->setWrapping(true);
		bb->addWidget(phi);	phi->setRange(-180, 180);	phi->setSingleStep(10);
		connect(phi, SIGNAL(valueChanged(int)), mgl, SLOT(setPhi(int)));
		connect(mgl, SIGNAL(phiChanged(int)), phi, SLOT(setValue(int)));
		phi->setToolTip(("Set value of \\phi angle."));
//printf(" 100 ");
	bb->addSeparator();
	}

	


	//printf("\nadding zooming menu \n");
	// zooming menu
	{
		oo = o->addMenu(("Zoom/move"));
		bb = new QToolBar(("Zoom graphics"),this);
		toolLayout->addWidget(bb, Qt::AlignTrailing);

//printf(" 10 ");
		a = new QAction(QPixmap(left_1_xpm), ("Move left"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(shiftLeft()));
		a->setToolTip(("Move graphics left by 1/3 of its width."));
		bb->addAction(a);		oo->addAction(a);

		//printf(" 11 ");

		a = new QAction(QPixmap(up_1_xpm), ("Move up"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(shiftUp()));
		a->setToolTip(("Move graphics up by 1/3 of its height."));
		bb->addAction(a);		oo->addAction(a);

		//printf(" 12 ");

		a = new QAction(QPixmap(zoom_1_xpm), ("Zoom in"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(zoomIn()));
		a->setToolTip(("Zoom in graphics."));
		bb->addAction(a);		oo->addAction(a);

		//printf(" 13 ");

		a = new QAction(QPixmap(norm_1_xpm), ("Zoom out"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(zoomOut()));
		a->setToolTip(("Zoom out graphics."));
		bb->addAction(a);		oo->addAction(a);

		//printf(" 14 ");

		a = new QAction(QPixmap(down_1_xpm), ("Move down"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(shiftDown()));
		a->setToolTip(("Move graphics down 1/3 of its height."));
		bb->addAction(a);		oo->addAction(a);

		//printf(" 15 ");

		a = new QAction(QPixmap(right_1_xpm), ("Move right"), this);
		connect(a, SIGNAL(triggered()), mgl, SLOT(shiftRight()));
		a->setToolTip(("Move graphics right by 1/3 of its width."));
		bb->addAction(a);		oo->addAction(a);

		//printf(" 100 \n");
	}

	

	// animation menu
	/*
	{
		o = Wnd->menuBar()->addMenu(_("Animation"));
		bb = new QToolBar(_("Animation"),Wnd);
		Wnd->addToolBar(Qt::LeftToolBarArea, bb);
		a = new QAction(QPixmap(next_sl_xpm), _("Next slide"), Wnd);
		Wnd->connect(a, SIGNAL(triggered()), mgl, SLOT(nextSlide()));
		a->setToolTip(_("Show next slide (Ctrl+.)."));
		a->setShortcut(Qt::CTRL+Qt::Key_Period);	o->addAction(a);		bb->addAction(a);
		a = new QAction(QPixmap(show_sl_xpm), _("Slideshow"), Wnd);
		a->setCheckable(true);
		Wnd->connect(a, SIGNAL(toggled(bool)), mgl, SLOT(animation(bool)));
		a->setToolTip(_("Run slideshow (CTRl+F5)."));
		a->setShortcut(Qt::CTRL+Qt::Key_F5);	o->addAction(a);		bb->addAction(a);
		a = new QAction(QPixmap(prev_sl_xpm), _("Prev slide"), Wnd);
		Wnd->connect(a, SIGNAL(triggered()), mgl, SLOT(prevSlide()));
		a->setToolTip(_("Show previous slide (Ctrl+,)."));
		a->setShortcut(Qt::CTRL+Qt::Key_Comma);	o->addAction(a);		bb->addAction(a);
	}
	*/

	menu_bar->addSeparator();
	o = menu_bar->addMenu(("Help"));
	o->addAction(("About"), mgl, SLOT(about()));
	o->addAction(("About Qt"), mgl, SLOT(aboutQt()));
	
	//return popup;
}


QMGL2::QMGL2(QWidget *parent) : QGLWidget(parent), gr(0)  {

}
QMGL2::~QMGL2() {
	if(gr)	{
		delete gr;
	}
}

void QMGL2::initializeGL()	{// recreate instance of MathGL core
	if(gr)	delete gr;
	gr = new mglGraph(1);	// use '1' for argument to force OpenGL output in MathGL
}

void QMGL2::resizeGL(int w, int h) {// standard resize replace
	QGLWidget::resizeGL(w, h);
	glViewport (0, 0, w, h);
}

void QMGL2::paintGL()  {	// main drawing function
	gr->Clf();	// clear previous OpenGL primitives
	gr->SubPlot(1,1,0);
	gr->Rotate(40,60);
	gr->Light(true);
	gr->AddLight(0,mglPoint(0,0,10),mglPoint(0,0,-1));
	gr->Axis();
	gr->Box();
	gr->FPlot("sin(pi*x)","i2");
	gr->FPlot("cos(pi*x)","|");
	gr->FSurf("cos(2*pi*(x^2+y^2))");
	gr->Finish();
	swapBuffers();	// show output on the screen
}

