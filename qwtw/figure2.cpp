
/**

	2D line view dialog.
	

	\file figure2.cpp
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/



#include "figure2.h"
#include <stdio.h>
#include <qwt_plot_grid.h>
//#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_rescaler.h>

#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_map.h>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QFocusEvent>
#include <QCheckBox>
#include <QToolButton>

#include "xmatrix2.h"
#include "sfigure.h"
#include "klifft/psd.h"
#include "xmutils.h"
#include "sqwlinetree.h"
#include <sstream>

int xmprintf(int level, const char* _Format, ...);


FSPicker::FSPicker(int xAxis, int yAxis, RubberBand rubberBand, 
				   DisplayMode trackerMode, QWidget* canv):
	QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canv) {

		/*  d_picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        d_plot->canvas() );
    d_picker->setStateMachine( new QwtPickerDragPointMachine() );
    d_picker->setRubberBandPen( QColor( Qt::green ) );
    d_picker->setRubberBand( QwtPicker::CrossRubberBand );
    d_picker->setTrackerPen( QColor( Qt::white ) );

		*/

}

QPointF FSPicker::transform1(	const QPoint & 	pos	 ) 	 const {
	QPointF ret = invTransform(pos);
	return ret;
}


FSPlot::FSPlot(QWidget *parent) : QwtPlot(parent), squareAxis(false) {
	//const bool doReplot = autoReplot();
	setAutoReplot(false);
	//setTitle("no titile yet");
	setCanvasBackground(QColor(Qt::white));

	// grid 
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(QPen(Qt::darkGray, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

	// panning with the left mouse button
    //( void ) new QwtPlotPanner( canvas );  // ?

	// zoom in/out with the wheel
    ( void ) new QwtPlotMagnifier( canvas() );

	enableAxis( QwtPlot::xBottom );  
	enableAxis( QwtPlot::yLeft );	
	
	setAxisScaleEngine( QwtPlot::xBottom, new QwtLinearScaleEngine );
	setAxisScaleEngine( QwtPlot::yLeft, new QwtLinearScaleEngine );

	legend.setFrameStyle(QFrame::Box|QFrame::Sunken);
    insertLegend(&legend, QwtPlot::TopLegend);

	//m_rescaler = new QwtPlotRescaler( canvas(), QwtPlot::xBottom, QwtPlotRescaler::Fixed );
   // m_rescaler->setAspectRatio( QwtPlot::yLeft, 1.0 );
	//m_rescaler->setEnabled(false);

	//setAutoReplot(doReplot); // ?
	setAutoReplot(false);
}

void FSPlot::resizeEvent(QResizeEvent* e) {
	QwtPlot::resizeEvent(e);
	if (squareAxis) {
		doSquareAxis();
	}
}

void FSPlot::setAxisSquare(bool square) {
	squareAxis = square;
	if (squareAxis) {
		doSquareAxis();
	}
}

void FSPlot::doSquareAxis() {
	const bool doReplot = autoReplot();
    setAutoReplot( false );

	QwtScaleMap smY = canvasMap(QwtPlot::yLeft);
	QwtScaleMap smX = canvasMap(QwtPlot::xBottom);

	double dxS = smX.sDist();
	double dyS = smY.sDist();

	double xScale = smX.pDist() / smX.sDist();
	double yScale = smY.pDist() / smY.sDist();


	if (xScale < yScale) { //  change Y scale
		double ysMiddle = (smY.s1() + smY.s2()) * 0.5;
		double dy = (smY.pDist() / xScale) * 0.5;
		setAxisScale(QwtPlot::yLeft,  ysMiddle - dy, ysMiddle + dy);
	} else { //  change X scale
		double xsMiddle = (smX.s1() + smX.s2()) * 0.5;
		double dx = (smX.pDist() / yScale) * 0.5;
		setAxisScale(QwtPlot::xBottom,  xsMiddle - dx, xsMiddle + dx);
	}

	setAutoReplot( doReplot );
	replot();
}

FigureItem::FigureItem(LineItemInfo*	info_, QwtPlotCurve*	line_) {
	info = info_;
	line = line_;
	if (info->mode == 0) {
		ma = 0;
	} else {
		ma = new QwtPlotMarker();
	}
}


FigureItem::~FigureItem() {

}

 TestScaleEngine::TestScaleEngine()    {
	setAttribute( QwtScaleEngine::Floating, true );
    setAttribute( QwtScaleEngine::Symmetric, true );
}
 
void  TestScaleEngine::autoScale( int maxNumSteps, double &x1, double &x2, double &stepSize ) const    {
    TestScaleEngine *that = const_cast<TestScaleEngine *>( this );
    that->setReference( 0.5 * ( x1 + x2 ) );
 
    QwtLinearScaleEngine::autoScale( maxNumSteps, x1, x2, stepSize );
}



Figure2::Figure2(const std::string& key_, XQPlots* pf_, QWidget * parent) : JustAplot(key_, pf_, parent, jQWT) {
	mode = 0;
	//cf = 0;
	tbModeChanging = false;
	clipperHost = false;
	setupUi();

	QIcon icon;
    icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::Off);
	icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/binokl.png")), QIcon::Normal, QIcon::On);
    setWindowIcon(icon);
	//setWindowIconText("qwtw");

	connect(tb1, SIGNAL(toggled( bool )), this, SLOT(ontb1( bool )));
	connect(tb2, SIGNAL(toggled( bool )), this, SLOT(ontb2( bool )));
	connect(tb3, SIGNAL(toggled( bool )), this, SLOT(ontb3( bool )));
	connect(tbSaveDataToTextFile, SIGNAL(clicked()), this, SLOT(onSaveData()));
	connect(tbSavePicture, SIGNAL(clicked()), this, SLOT(onSaveImage()));
	connect(tbFFT, SIGNAL(clicked()), this, SLOT(onTbFFT()));
	connect(tbSquareAxis, SIGNAL(toggled(bool)), this, SLOT(onTbSquareAxis(bool)));
	connect(tbClip, SIGNAL(toggled(bool)), this, SLOT(onClip(bool)));
	connect(tbResetLayout, SIGNAL(clicked()), this, SLOT(onResetLayout()));

	panner = new QwtPlotPanner(plot1->canvas());
    panner->setMouseButton(Qt::LeftButton);
	panner->setCursor(Qt::OpenHandCursor);
	panner->setEnabled(false);

	 picker = new FSPicker(QwtPlot::xBottom, QwtPlot::yLeft,
       // QwtPicker::PointSelection | QwtPicker::DragSelection, 
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, 
        plot1->canvas());

	// picker->setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);

    picker->setStateMachine(new QwtPickerDragPointMachine());
    picker->setRubberBandPen(QColor(Qt::green));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerPen(QColor(Qt::darkBlue));
	picker->setTrackerMode(QwtPicker::ActiveOnly);
	picker->setEnabled(false);
	//picker->setParent(plot1);
	//connect(picker, SIGNAL(changed(const QwtPolygon&)), this, SLOT(onPickerSelection(const QwtPolygon&)));
	
	connect(picker, SIGNAL(selected(const QPolygon&)), this, SLOT(onPickerSelection(const QPolygon&)));
	connect(picker, SIGNAL(moved(const QPoint&)), this, SLOT(onPickerMove(const QPoint&)));
	//connect(picker, SIGNAL(appended(const QwtPolygon&)), this, SLOT(onPickerSelection(const QwtPolygon&)));

	zoomer = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, plot1->canvas());

	zoomer->setEnabled(true);
	zoomer->setTrackerMode( QwtPicker::ActiveOnly );
    zoomer->setTrackerPen( QColor( Qt::black ) );
	zoomer->zoom( 0 );

	tb3->setChecked(true);

	//   test: =========
	QWidget *w1 = panner->parentWidget();
	QWidget *w2 = picker->parentWidget();
	QWidget *w3 = zoomer->parentWidget();
   // zoomer->setRubberBand(QwtPicker::RectRubberBand);
   // zoomer->setRubberBandPen(QColor(Qt::green));
    //zoomer->setTrackerMode(QwtPicker::ActiveOnly);
    //zoomer->setTrackerPen(QColor(Qt::darkBlue));


//	setCentralWidget(plot1);
	
	title(key.c_str());

	setFocusPolicy(Qt::StrongFocus);
	titleFont.setBold(false); titleFont.setPointSize(10);
	axisFont.setBold(false); axisFont.setPointSize(8);

	plot1->setAxisFont(0, axisFont);
	plot1->setAxisFont(1, axisFont);
	plot1->setAxisFont(2, axisFont);

	plot1->legend.setFont(axisFont);

	vLineMarker.setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
	vLineMarker.setLabelOrientation(Qt::Vertical);
	vLineMarker.setLineStyle(QwtPlotMarker::VLine);
	vLineMarker.setLinePen(QPen(Qt::yellow));
	vLineMarker.setVisible(false);
	vLineMarker.attach(plot1);

	//plot1->setAxisScaleEngine(QwtPlot::yLeft, (QwtScaleEngine*)(new TestScaleEngine()));
	//plot1->setAxisScaleEngine(QwtPlot::xBottom, (QwtScaleEngine*)(new TestScaleEngine()));

	makeMarkersVisible(true);
}

Figure2::~Figure2() {
	removeLines();
	//emit exiting(key);

	//if (panner != 0) { delete panner; panner = 0; }
	//if (picker != 0) { delete picker; picker = 0; }
	//if (zoomer != 0) { delete zoomer; zoomer = 0; }
}

void Figure2::ontb1(bool checked ) {  //   picker
	picker->setEnabled(checked);
	pf->setAllMarkersVisible(checked);

	if (checked) {
		mode = 1;
	} else {
		mode = 0;
	}

	setTBState(); 
}

void Figure2::onResetLayout() {
	if (lines.empty()) {
		return;
	}

	// do 'vertical scale':
	double yMax = -BIGNUMBER, yMin = BIGNUMBER;
	double xMax = -BIGNUMBER;
	double xMin = BIGNUMBER;
	bool ok = false;
	int n = 0;
	for (std::list<FigureItem*>::iterator it = lines.begin(); it != lines.end(); it++) {
		FigureItem* fi = *it;
		LineItemInfo* i = fi->info;
		if (!i->important) {
			continue;
		}
		n = i->size;

		if (i->mode != 3) { //   'simple' line
			if (xMax < i->x[n - 1]) xMax = i->x[n - 1];
			if (xMin > i->x[0]) xMin = i->x[0];
		} else { //  i->more == 3:  looks like a top view plot
			if (i->time == 0) {
				mxat(false);
				continue;
			}
			for (long long k = 0; k < n; k++) {
				if (xMax < i->x[k]) xMax = i->x[k];
				if (xMin > i->x[k]) xMin = i->x[k];
			}
		}

		for (long long k = 0; k < n; k++) {
			if (yMax < i->y[k]) {
				yMax = i->y[k];
			}
			if (yMin > i->y[k]) {
				yMin = i->y[k];
			}
		}
		ok = true;
	}
	//plt->setAxisScale(xAxis(), x1, x2);
	//plot1->setAxisScale(plot1->xBottom, xMin, xMax);
	//plot1->setAxisScale(plot1->yLeft, yMin, yMax);
	if (!ok) {
		return;
	}

	QRectF zr(xMin, yMin, xMax - xMin, yMax - yMin);
	zoomer->zoom(zr);

	plot1->replot(); // ?
}

void Figure2::onClip(bool checked) {
	//  left and right points:
	clipperHost = true;
	QwtScaleMap smX = plot1->canvasMap(QwtPlot::xBottom);
	double  x1 = smX.s1();
	double  x2 = smX.s2();
	

	pf->clipAll(x1, x2);
}

void Figure2::ontb2(bool checked ) { //   panner
	panner->setEnabled(checked);
	if (checked) {
		mode = 2; 
		//plot1->setCursor(Qt::OpenHandCursor);

	} else {
		mode = 0;
	}
	setTBState();
}
void Figure2::ontb3(bool checked ) {  //  zoomer
	zoomer->setEnabled(checked);
	//zoom(0);
	if (checked) mode = 3; else mode = 0;
	setTBState();
}

void Figure2::setTBState() {
	if (tbModeChanging) return;
	tbModeChanging = true;
	switch(mode) {
	case 0: tb1->setChecked(false);  tb2->setChecked(false);  tb3->setChecked(false);  break;
	case 1:                          tb2->setChecked(false);  tb3->setChecked(false);  break;
	case 2: tb1->setChecked(false);                           tb3->setChecked(false);  break;
	case 3: tb1->setChecked(false);  tb2->setChecked(false);                           break;
	};
	tbModeChanging = false;
}

void Figure2::title(const std::string& s) {
	sTitle = s;

	QwtText title(s.c_str());
	title.setFont(titleFont);
	plot1->setTitle(title);

	JustAplot::title(s);

	zoomer->zoom(0);
	//plot1->setFooter(s.c_str());
}
void Figure2::footer(const std::string& s) {
	QwtText foo(s.c_str());
	foo.setFont(axisFont);

	plot1->setFooter(foo);
}

void Figure2::xlabel(const std::string& s) {
	QwtText title(s.c_str());
	title.setFont(axisFont);
	plot1->setAxisTitle(plot1->xBottom, title);
}

void Figure2::ylabel(const std::string& s) {
	QwtText title(s.c_str());
	title.setFont(axisFont);
	plot1->setAxisTitle(plot1->yLeft, title);
}
 
void Figure2::addLine(LineItemInfo* line) {
	const bool doReplot = plot1->autoReplot();
	plot1->setAutoReplot(false);
	JustAplot::addLine(line);
	QwtPlotCurve* cl = new QwtPlotCurve(line->legend.c_str());
	FigureItem* xd = new FigureItem(line, cl);

	if (xd->info->mode != 0) {
		xd->ma->setVisible(true);
		xd->ma->attach(plot1);
	}
	lines.push_back(xd);
	
	//  set default values:
	QwtSymbol* sym = new QwtSymbol();
	sym->setStyle(QwtSymbol::NoSymbol);
	sym->setPen(QColor(Qt::black));
	sym->setSize(line->symSize);
	sym->setBrush(QColor(Qt::darkBlue)); 
	
	QPen pen;
	cl->setStyle(QwtPlotCurve::Lines);
    cl->setTitle(line->legend.c_str());
	pen.setColor(QColor(Qt::darkBlue));
	pen.setWidth(line->lineWidth);
	
	QColor color = Qt::black;

	if (line->style == std::string()) {
		//
	} else {
		int sn = line->style.size();
		
		if (sn > 0) { //    last is always color:
			//  set color:
			switch (line->style[sn - 1]) {
				case 'r':  color = Qt::red;  break;
				case 'd': color = Qt::darkRed;	break;
				case 'k': color = Qt::black;  break;
				case 'w': color = Qt::white;  break;
				case 'g': color = Qt::green;  break;
				case 'G': color = Qt::darkGreen;  break;
				case 'm': color = Qt::magenta;   break;
				case 'M': color = Qt::darkMagenta;  break;
				case 'y': color = Qt::yellow;   break;
				case 'Y': color = Qt::darkYellow;  break;
				case 'b': color = Qt::blue;  break;
				case 'c': color = Qt::cyan;  break;
				case 'C': color = Qt::darkCyan;  break;
			};

			sym->setBrush(color);  
			sym->setPen(color); 
			pen.setColor(color); 

		}

		if (sn > 1) {  //  first is always a line style:
			switch (line->style[0]) {
			case ' ': cl->setStyle(QwtPlotCurve::NoCurve); break;
			case '-': cl->setStyle(QwtPlotCurve::Lines); break;
			case '%': cl->setStyle(QwtPlotCurve::Sticks); break;
			case '#': cl->setStyle(QwtPlotCurve::Steps); break;
			case '.': cl->setStyle(QwtPlotCurve::Dots); break;
			};

		}
		if (sn == 3) {  //   middle is symbol type
			switch (line->style[1]) {
			case 'e': sym->setStyle(QwtSymbol::Ellipse);  break;
			case 'r': sym->setStyle(QwtSymbol::Rect);  break;
			case 'd': sym->setStyle(QwtSymbol::Diamond);  break;
			case 't': sym->setStyle(QwtSymbol::Triangle);  break;
			case 'x': sym->setStyle(QwtSymbol::Cross);  break;
			case 's': sym->setStyle(QwtSymbol::Star1);  break;
			case 'q': sym->setStyle(QwtSymbol::Star2);  break;
			case 'w': sym->setStyle(QwtSymbol::XCross);  break;
			case 'u': sym->setStyle(QwtSymbol::UTriangle);  break;
			};

		}

	}

	cl->setCurveFitter(NULL);
	if (sym->style() == QwtSymbol::NoSymbol) {
		//cl->setSymbol(0);
		delete sym; //   we do not need it anymore
		cl->setLegendAttribute(QwtPlotCurve::LegendShowLine);
		
	} else {
		cl->setSymbol(sym); //    "cl" will delete it itself later
		cl->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
		//QwtSymbol* sym1 = new QwtSymbol(sym);
		//cl->setSymbol(sym1);
	}

/*
	switch (xd->info->mode) {
		case 0: break;
		case 1:
		case 2:
			xd->ma.setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
			xd->ma.setLabelOrientation(Qt::Vertical);
			xd->ma.setLineStyle(QwtPlotMarker::VLine);
			xd->ma.setLinePen(pen);
			break;

		case 3: {
			QwtSymbol* pms = new QwtSymbol();
			pms->setStyle(QwtSymbol::Diamond);
			pms->setPen(QColor(Qt::red));
			pms->setSize(10);
			pms->setBrush(QColor(Qt::darkBlue));

			xd->ma.setSymbol(pms);
			}
			break;
	};

	*/
	cl->setPen(pen);

    //cl->setSamples(line->x, line->y, line->size);
    cl->setRawSamples(line->x, line->y, line->size);

	cl->setYAxis( QwtPlot::yLeft );  cl->setXAxis( QwtPlot::xBottom );

	cl->setPaintAttribute(QwtPlotCurve::ClipPolygons, true );
	cl->setPaintAttribute(QwtPlotCurve::FilterPoints, true );
	cl->setPaintAttribute(QwtPlotCurve::MinimizeMemory, false);

	cl->attach(plot1);

	if (xd->info->mode != 0) {

		pen.setWidth(2);
		QwtSymbol* pms = new QwtSymbol();
		pms->setStyle(QwtSymbol::Diamond);
		pen.setColor(Qt::darkBlue);
		pms->setPen(pen);
		pms->setSize(10);
		pms->setBrush(color); 
		
		xd->ma->setSymbol(pms);
	}

	plot1->updateAxes();
	//zoomer->setZoomBase(true);
	plot1->setAutoReplot(doReplot);
	plot1->replot();
	zoomer->setZoomBase(false);
}

void Figure2::setupUi()     {
    if (this->objectName().isEmpty())
        this->setObjectName(QString::fromUtf8("Figure2"));
    this->resize(400, 300);
    QFont font;
    font.setFamily(QString::fromUtf8("MS Sans Serif"));
    font.setPointSize(12);
    this->setFont(font);
    this->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(2);
    verticalLayout->setMargin(2);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    top_frame = new QFrame(this);
    top_frame->setObjectName(QString::fromUtf8("top_frame"));
    top_frame->setMinimumSize(QSize(0, 32));
    top_frame->setFrameShape(QFrame::NoFrame);
    top_frame->setFrameShadow(QFrame::Raised);
    top_frame->setLineWidth(1);
    horizontalLayout = new QHBoxLayout(top_frame);
    horizontalLayout->setSpacing(2);
    horizontalLayout->setMargin(2);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

    tb1 = new QToolButton(top_frame);
    tb1->setObjectName(QString::fromUtf8("tb1"));
	ui_addTBIcon(tb1, ":/icons/arrow.png");
	tb1->setText("ARROW");
	tb1->setToolTip("ARROW");
	//icon1.addPixmap(QPixmap(QString::fromUtf8(":/icons/arrow.PNG")), QIcon::Normal, QIcon::Off);
	//icon1.addFile("arrow.PNG");
    //tb1->setIcon(icon1);

	tb1->setCheckable(true);
    horizontalLayout->addWidget(tb1);

    tb2 = new QToolButton(top_frame);
    tb2->setObjectName(QString::fromUtf8("tb2"));
	tb2->setText("PAN");
	tb2->setToolTip("PAN");

	ui_addTBIcon(tb2, ":/icons/pan.png");
	tb2->setCheckable(true);
    horizontalLayout->addWidget(tb2);

    tb3 = new QToolButton(top_frame);
    tb3->setObjectName(QString::fromUtf8("tb3"));
	tb3->setText("ZOOM");
	tb3->setToolTip("ZOOM");

	ui_addTBIcon(tb3, ":/icons/zoom.png");
	tb3->setCheckable(true);
    horizontalLayout->addWidget(tb3);

	tbSaveDataToTextFile = new QToolButton(top_frame);
	tbSaveDataToTextFile->setToolTip("Save data to text file");
	ui_addTBIcon(tbSaveDataToTextFile, ":/icons/floppy.png");
	horizontalLayout->addWidget(tbSaveDataToTextFile);

	tbSavePicture = new QToolButton(top_frame);
	tbSavePicture->setToolTip("Save data  as image file");
	ui_addTBIcon(tbSavePicture, ":/icons/savecontent.png");
	horizontalLayout->addWidget(tbSavePicture);

	tbFFT  = new QToolButton(top_frame);
	tbFFT->setToolTip("see FFT");
	tbFFT->setText("f");
	horizontalLayout->addWidget(tbFFT);

	tbSquareAxis  = new QToolButton(top_frame);
	tbSquareAxis->setToolTip("make axis square");
	tbSquareAxis->setText("[]");
	tbSquareAxis->setCheckable(true);
	horizontalLayout->addWidget(tbSquareAxis);

	tbClip = new QToolButton(top_frame);
	tbClip->setToolTip("clip");
	tbClip->setText("clip");
	ui_addTBIcon(tbClip, ":/icons/wireframe.png");
	tbClip->setCheckable(true);
	horizontalLayout->addWidget(tbClip);

	tbResetLayout = new QToolButton(top_frame);
	tbResetLayout->setToolTip("reset zooming");
	tbResetLayout->setText("reset");
	ui_addTBIcon(tbResetLayout, ":/icons/thunderstorm-icon.png");
	//tbResetLayout->setCheckable(true);
	horizontalLayout->addWidget(tbResetLayout);

    horizontalSpacer = new QSpacerItem(244, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);
    verticalLayout->addWidget(top_frame);

    plot1 = new FSPlot(this);
    plot1->setObjectName(QString::fromUtf8("plot1"));

    verticalLayout->addWidget(plot1);
    retranslateUi();
    QMetaObject::connectSlotsByName(this);
} // setupUi

void Figure2::retranslateUi()     {
 //   setWindowTitle(QApplication::translate("figure2", "figure 1", 0, QApplication::UnicodeUTF8));
 //   tb1->setText(QApplication::translate("figure2", "...", 0, QApplication::UnicodeUTF8));
 //   tb2->setText(QApplication::translate("figure2", "...", 0, QApplication::UnicodeUTF8));
  //  tb3->setText(QApplication::translate("figure2", "...", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(this);
} // retranslateUi 

void Figure2::ui_addTBIcon(QToolButton* tb, const char* i) {
    QIcon icon;
	QString s = QString::fromUtf8(i);
	QPixmap pm = QPixmap(s);
    icon.addPixmap(pm, QIcon::Normal, QIcon::Off);

//	icon.addFile(":/icons/binokl");
    tb->setIcon(icon);
}

void Figure2::removeLines() {
	std::list<FigureItem*>::iterator xi = lines.begin();
	while (xi != lines.end()) {
		FigureItem* cf = *xi;
		cf->line->setData(NULL);
		cf->line->detach();
		//   error? delete cf;

		delete cf->line;
		xi++;
	}
	lines.clear();
	//cf = 0;
}

void Figure2::closeEvent ( QCloseEvent * event ) {
	removeLines();
}

void Figure2::onTbSquareAxis(bool checked) {
	plot1->setAxisSquare(checked);
	zoomer->keepEqual(checked);
}

void Figure2::onTbFFT() {
	//"onTbFFT()\n");
	
	SLDialog dlg(lines, this);
	dlg.exec();
	std::list<FigureItem*>	selected = dlg.getSelectedLines();
	if (selected.size() == 0) {
		return;
	}
	unsigned int ws = dlg.getWindowSize();
	char stmp[16];
	sprintf(stmp, "%u", ws);

	//  left and right points:
	QwtScaleMap smX = plot1->canvasMap(QwtPlot::xBottom);
	double  x1 = smX.s1();  
	double  x2 = smX.s2();
	std::ostringstream s; s.precision(2); s << std::fixed << " time " << x1 << " - " << x2;

	std::string pn = this->sTitle; pn.append(" FFT "); pn.append(stmp);
	std::string pKey = this->key; pKey.append("_fft_"); pKey.append(stmp); pKey.append(s.str());

    pf->figure(pKey);
    pf->title(pn);
	pf->xlabel("Hz");

	pf->footer(s.str());

	std::list<FigureItem*>::iterator it;
	for (it = selected.begin(); it != selected.end(); it++) {
		LineItemInfo* info = (*it)->info;
		// 1. get start and end indexes: use only visible part
		mxat(info->size > 0);
		size_t ix1 = findClosestPoint_1(0, info->size - 1, info->x, x1);
		size_t ix2 = findClosestPoint_1(0, info->size - 1, info->x, x2);

		//  2. calculate FFT:
		double periodOfSampling = (info->x[ix2] - info->x[ix1]) / ((double)(ix2 - ix1));
		Psd psd;
        psd.compute(info->y + ix1, ix2 - ix1, periodOfSampling, ws); // 64

		//  3. draw this fft:
		std::string ke = info->legend; ke.append("_fft_");
        pf->plot(psd.frequencies, psd.psd, psd.size, ke.c_str(), info->style.c_str(), 1, 1);
	}
}

void Figure2::onSaveImage() {
	//QPixmap qPix = QPixmap::grabWidget(plot1);
	QPixmap qPix = plot1->grab();
	if (qPix.isNull()) {
		qDebug("Failed to capture the plot for saving");
		return;
	}
	QString types("Portable Network Graphics file (*.png);;");
	QString filter;							// Type of filter
	QString jpegExt = ".jpeg", pngExt = ".png", tifExt = ".tif", bmpExt = ".bmp", tif2Ext = "tiff";		// Suffix for the files
	//QString suggestedName = restorePath().replace("flxhst", "jpeg");

	QString suggestedName = sTitle.c_str();

	QString fn = QFileDialog::getSaveFileName(this, tr("Save Image"), suggestedName, types, &filter);
	if (!fn.isEmpty()) {
		qPix.save(fn, "png");
	}

}

void Figure2::onSaveData() {
    size_t n = lines.size();
    if(n == 0) return;
    if(sTitle.size() < 1) return;

    size_t i, w;
    std::list<FigureItem*>::iterator xi = lines.begin();
    for(xi = lines.begin(); xi != lines.end(); xi++) {
	   FigureItem* cf = *xi;
	   w = cf->info->size;
	   if(w < 1) {
		  continue;
	   }
	   std::string fn = sTitle + cf->info->legend + ".csv";
	   FILE*	f = fopen(fn.c_str(), "wt");
	   if(f == NULL) return;
	   for(i = 0; i < w; i++) {
		  fprintf(f, "%.9G\t%.9G\n", cf->info->x[i], cf->info->y[i]);
	   }
	   fclose(f);
    }
}


void Figure2::onPickerSelection(const QPolygon& pa) {
    int x = pa.at(0).x();
    int y = pa.at(0).y();

    onPickerSignal(x, y);
    setWindowTitle(sTitle.c_str());
}

void Figure2::onPickerMove(const QPoint& pos) {
    onPickerSignal(pos.x(), pos.y());
}

void Figure2::focusInEvent(QFocusEvent * event) {
    QDialog::focusInEvent(event);
    if(event->gotFocus()) {
	   emit onSelection(key);
    }
}

void Figure2::onPickerSignal(int x, int y) {
    if(lines.size() == 0) return;

    QPointF p = picker->transform1(QPoint(x, y));
    double xx = p.x();
    double yy = p.y();

   
    

    emit onSelection(key);

   // emit onPicker(key, p.x(), p.y());
    std::list<FigureItem*>::iterator it = lines.begin();
    FigureItem* mfi = (*it);
    long long minIndex = 0, index = 0;
    double minDist = findDistance(mfi->info, xx, yy, minIndex);
    if (minIndex == 0xffffffff) { //  error?
	    xmprintf(2, "Figure2::onPickerSignal() failed\n");
	    return;
    }
    it++;
    bool ok = true;
    while (it != lines.end()) {
	    FigureItem* fi = *it;
	    LineItemInfo* i = fi->info;
	    size_t size = i->size;
	    if ((i->mode == 0) || (size < 2)) continue;
	    double dist = findDistance(i, xx, yy, index);
	    if ((index != 0xffffffff) && (dist < minDist)) {
		    minDist = dist;
		    minIndex = index;
		    mfi = fi;
	    }
	    it++;
    }
    double t = 0.;
    if (mfi->info->mode == 3) {
	    t = mfi->info->time[minIndex];
    } else {
	    t = mfi->info->x[minIndex];
    }
    
   
    double xxm = mfi->info->x[minIndex];
    double yym = mfi->info->y[minIndex];
    //std::ostringstream s; 
    //s << xxm << ", " << yym << ", (" << mfi->info->legend << ")";
    char s[256];
    sprintf(s, "%.6f, %.6f (%s), index=%lld", xxm, yym, mfi->info->legend.c_str(), minIndex);

   // setWindowTitle(s.str().c_str());
    setWindowTitle(s);
    
    if(ok) {
	   pf->drawAllMarkers(t);
    }
}

void Figure2::drawMarker(double X, double Y, int type) {
    double xx = X;
    double yy = Y;

    for (std::list<FigureItem*>::iterator it = lines.begin(); it != lines.end(); it++) {
	    FigureItem* fi = *it;
	    if (fi->info->mode == 0)  continue;
	   // fi->ma->setVisible(true);
	    fi->ma->setValue(xx, yy);
    }

 //   plot1->replot(); // ?
}

void Figure2::makeMarkersVisible(bool visible) {
	JustAplot::makeMarkersVisible(visible);
	if (lines.empty()) {
		return;
	}

	int mode = -1;

	for (std::list<FigureItem*>::iterator it = lines.begin(); it != lines.end(); it++) {
		FigureItem* fi = *it;
		if (fi->info->mode == 0) {
			continue;
		}
		fi->ma->setVisible(visible);
		if (mode < 0) mode = fi->info->mode; //    select mode of the first line
	}
	if (mode < 3) {
		vLineMarker.setVisible(visible);
	}

}

void Figure2::replot() {
	plot1->replot();

}

void Figure2::drawMarker(double t) {
	JustAplot::drawMarker(t);

    if(lines.empty()) {
	   return;
    }

    int mode = -1;

    for (std::list<FigureItem*>::iterator it = lines.begin(); it != lines.end(); it++) {
	    FigureItem* fi = *it;
		if (fi->info->mode == 0) {
			continue;
		}

	   // fi->ma->setVisible(true);


	    fi->ma->setValue(fi->info->x[fi->info->ma.index], fi->info->y[fi->info->ma.index]);


	    if (mode < 0) mode = fi->info->mode; //    select mode of the first line
    }
    if (mode < 3) {
	   // vLineMarker.setVisible(true);
	   vLineMarker.setValue(t, 0);
    }

   // plot1->replot(); // ?
}

void Figure2::onClip(double t1, double t2) {
	JustAplot::onClip(t1, t2);
	if (lines.empty()) {
		return;
	}
	if (clipperHost) {
		clipperHost = false;
		return;
	}

	bool ok = false;

	// do 'vertical scale':
	double yMax = -BIGNUMBER, yMin = BIGNUMBER;
	double xMax = -BIGNUMBER;
	double xMin = BIGNUMBER;

	for (std::list<FigureItem*>::iterator it = lines.begin(); it != lines.end(); it++) {
		FigureItem* fi = *it;
		LineItemInfo* i = fi->info;
		if (!i->important) {
			continue;
		}
		if (i->mode != 3) { //   'simple' line
			if (xMax < t2) xMax = t2;
			if (xMin > t1) xMin = t1;

			//  bounds check:
			if (i->x[0] > t2) continue;
			if (i->x[i->size-1] < t1) continue;
			mxat(i->size > 0);
			long long x1 = findClosestPoint_1(0, i->size - 1, i->x, t1);
			mxat(i->size > x1);
			long long x2 = findClosestPoint_1(x1, i->size - 1, i->x, t2);
			for (long long k = x1; k < x2; k++) {
				if (yMax < i->y[k]) { yMax = i->y[k]; 	}
				if (yMin > i->y[k]) { yMin = i->y[k];	}
			}
			ok = true;
		} else { //  i->more == 3:  looks like a top view plot
			if (i->time == 0) {
				mxat(false);
				continue;
			}
			mxat(i->size > 0);
			long long i1 = findClosestPoint_1(0, i->size - 1, i->time, t1);
			mxat(i->size > i1);
			long long i2 = findClosestPoint_1(i1, i->size - 1, i->time, t2);

			for (long long k = i1; k < i2; k++) {
				if (yMax < i->y[k]) {
					yMax = i->y[k];
				}
				if (yMin > i->y[k]) {
					yMin = i->y[k];
				}
				if (xMax < i->x[k]) xMax = i->x[k];
				if (xMin > i->x[k]) xMin = i->x[k];
			}
			ok = true;
		}
	}
	//plt->setAxisScale(xAxis(), x1, x2);
	//plot1->setAxisScale(plot1->xBottom, xMin, xMax);
	//plot1->setAxisScale(plot1->yLeft, yMin, yMax);

	QRectF zr(xMin, yMin, xMax - xMin, yMax - yMin);
	zoomer->zoom(zr);

	plot1->replot(); // ?
}

void Figure2::setAxesEqual() {
	zoomer->keepEqual(true);
}

SLDialog::SLDialog(std::list<FigureItem*> lines, QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);
	QVBoxLayout* vbl = new QVBoxLayout(ui.saFrame);
	char stmp[16];


	unsigned int ws = 1;
	do {
		sprintf(stmp, "%u", ws);
		ui.cbWindowSize->addItem(stmp);

		ws <<= 1;
	} while (ws <= 512);

	std::list<FigureItem*>::iterator it;
	for (it = lines.begin(); it != lines.end(); it++) {
		QCheckBox *cb = new QCheckBox((*it)->info->legend.c_str(), this);
		items.insert(std::make_pair(cb, *it));
		
		vbl->addWidget(cb);
	}
}

std::list<FigureItem*>    SLDialog::getSelectedLines() {
	std::list<FigureItem*> ret;
	std::map<QCheckBox*, FigureItem*>::iterator it;
	for (it = items.begin(); it != items.end(); it++) {
		QCheckBox *cb = it->first;
		if (cb->isChecked()) {
			ret.push_back(it->second);
		}
	}
	return ret;
}

unsigned int SLDialog::getWindowSize() {
	return ui.cbWindowSize->currentText().toInt();
}

std::list<FigureItem*>	Figure2::selectLines() {
	//std::list<FigureItem*> ret;
	SLDialog dlg(lines, this);
	dlg.exec();
	return dlg.getSelectedLines();
	//return ret;
}

Zoomer::Zoomer(int xAxis, int yAxis, QWidget *canvas): shouldKeepAxesEqual(false), 
    QwtPlotZoomer(xAxis, yAxis, canvas)  {

   // setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    setTrackerMode(QwtPicker::AlwaysOff);
   // setRubberBand(QwtPicker::NoRubberBand);
	setRubberBand(QwtPicker::RectRubberBand);
	setRubberBandPen(QColor(Qt::darkBlue));

	initMousePattern(2);
	setResizeMode(Stretch);

    // RightButton: zoom out by 1

    // Ctrl+RightButton: zoom out to full size

   // setMousePattern(QwtEventPattern::MouseSelect2,        Qt::RightButton, Qt::ControlModifier);
   // setMousePattern(QwtEventPattern::MouseSelect3,      Qt::RightButton);

	setMousePattern(QwtEventPattern::MouseSelect2,     Qt::RightButton, Qt::ControlModifier);
    setMousePattern(QwtEventPattern::MouseSelect3,     Qt::RightButton);
}
void Zoomer::keepEqual(bool e) {
	if (shouldKeepAxesEqual != e) {
		shouldKeepAxesEqual = e;
	}
}

void Zoomer::rescale() {
	//QwtPlotZoomer::rescale();
	//return;

    QwtPlot *plt = plot();
    if ( !plt )
        return;

    const QRectF &rect = zoomRect();
    if ( rect != scaleRect() )
    {
        const bool doReplot = plt->autoReplot();
        plt->setAutoReplot( false );

        double x1 = rect.left();
        double x2 = rect.right();
        double y1 = rect.top();
        double y2 = rect.bottom();

		if (shouldKeepAxesEqual == false) {
			if ( !plt->axisScaleDiv( xAxis() ).isIncreasing() )
				qSwap( x1, x2 );
			plt->setAxisScale( xAxis(), x1, x2 );

			if ( !plt->axisScaleDiv( yAxis() ).isIncreasing() )
				qSwap( y1, y2 );
				plt->setAxisScale( yAxis(), y1, y2 );
		} else {
			QwtScaleMap smY = plt->canvasMap(QwtPlot::yLeft);
			QwtScaleMap smX = plt->canvasMap(QwtPlot::xBottom);

			double xScale = smX.pDist() / fabs(x2 - x1);
			double yScale = smY.pDist() / fabs(y2 - y1);

			if (xScale < yScale) { //  change Y scale
				double ysMiddle = (y1 + y2) * 0.5;
				double dy = (smY.pDist() / xScale) * 0.5;
				plt->setAxisScale(QwtPlot::yLeft,  ysMiddle - dy, ysMiddle + dy);
				plt->setAxisScale(QwtPlot::xBottom,  x1, x2);
			} else { //  change X scale
				double xsMiddle = (x1 + x2) * 0.5;
				double dx = (smX.pDist() / yScale) * 0.5;
				plt->setAxisScale(QwtPlot::xBottom,  xsMiddle - dx, xsMiddle + dx);
				plt->setAxisScale(QwtPlot::yLeft, y1, y2 );
			}
		}

        plt->setAutoReplot( doReplot );

        plt->replot();
    }
}


