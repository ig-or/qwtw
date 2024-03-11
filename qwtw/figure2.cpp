
/**

	2D line view dialog.
	

	\file figure2.cpp
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/



#include "figure2.h"
#include "settings.h"
#include <stdio.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
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
#include <QPushButton>
#include <QToolButton>
#include <QColorDialog>

#include "xmatrix2.h"
#include "sfigure.h"
#include "klifft/psd.h"
#include "xmutils.h"
#include "sqwlinetree.h"
#include "line.h"
#include <sstream>

int xmprintf(int level, const char* _Format, ...);
static int markerID = 1;


FigureItem::FigureItem(LineItemInfo* info_, QwtPlotCurve* line_) {
	info = info_;
	line = line_;
	//key = key_;
	if (info->mode == 0) {
		ma = 0;
	}
	else {
		ma = new QwtPlotMarker();
	}
	id = info_->id;
	if (!info->lCheck()) {
		xmprintf(4, "FigureItem::FigureItem: bad line added \n");
	}
}

FigureItem::~FigureItem() {
	if (ma != 0) {
		delete ma;
	}
}




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

QwtText 	FSPicker::trackerTextF(const QPointF& pos) const {
	QString text;
	int dn = qwSettings.pickerDigitsNumber;
	text = QString::number(pos.x(), 'f', dn)	+ ", " + QString::number(pos.y(), 'f', dn);

	return QwtText(text);
}

VLineMarker::VLineMarker(const char* text, double time, int id_): t(time), QWMarker(id_) {
	QwtText label(text);
	label.setFont(QFont("Consolas", 12, QFont::Bold));
	label.setBackgroundBrush(QBrush(QColor(250, 250, 250)));
	QPen pen( Qt::black, 1 );
	label.setBorderPen(pen);

	setLabel(label);
    setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
    setLabelOrientation( Qt::Vertical );
    setLineStyle( QwtPlotMarker::VLine );
    setLinePen( Qt::black, 2, Qt::DashDotLine );
    setXValue( time );
}

AMarker::AMarker(const char* text, double x_, double y_, const QColor& color_, int id_): QWMarker(id_)	 {
	amInit(text, x_, y_, color_, amBottomRight);
}
AMarker::AMarker(const char* text, double x_, double y_, const QColor& color_, AMarker::AMPos pos_, int id_): QWMarker(id_)	 {
	amInit(text, x_, y_, color_, pos_);
}
void AMarker::amInit(const char* text, double x_, double y_, const QColor& color_,  AMPos pos_) {
	x = x_;
	y = y_;
	color = color_;
	pos = pos_;

	QwtText label(text);
	label.setFont(QFont("Consolas", 12, QFont::Bold));

	//QColor co = qwSettings.markerColor();

	label.setColor(color);
	label.setBackgroundBrush(QBrush(QColor(250, 250, 250)));
	QPen pen( color, 1 );
	label.setBorderPen(pen);

	setRenderHint( QwtPlotItem::RenderAntialiased, true );
    setItemAttribute( QwtPlotItem::Legend, false );
	setLabel( label );
	ArrowSymbol* as;
	switch (pos) {

	case amTopLeft:  
		as =  new ArrowSymbol(150.0, 14);
		setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
		break;

	case amTopRight:
		as =  new ArrowSymbol(-150.0, 14);
		setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
		break;

	case amBottomLeft:
		as =  new ArrowSymbol(30.0, 14);
		setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
		break;

	case amBottomRight:  
	default:
		as =  new ArrowSymbol(); 
		setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
		break;
	};
    setSymbol( as );
    setValue( QPointF( x, y));
}


FSPlot::FSPlot(QWidget *parent, unsigned int flags_) : QwtPlot(parent), squareAxis(false) {
	flags = flags_;
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
	
	if (flags & 1) {
		setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
	}	else {
		setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);
	}
	if (flags & 2) {
		setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
	} 	else {
		setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
	}

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


 TestScaleEngine::TestScaleEngine()    {
	setAttribute( QwtScaleEngine::Floating, true );
	setAttribute( QwtScaleEngine::Symmetric, true );
}
 
void  TestScaleEngine::autoScale( int maxNumSteps, double &x1, double &x2, double &stepSize ) const    {
	TestScaleEngine *that = const_cast<TestScaleEngine *>( this );
	that->setReference( 0.5 * ( x1 + x2 ) );
 
	QwtLinearScaleEngine::autoScale( maxNumSteps, x1, x2, stepSize );
}


SelectInfoDlg::SelectInfoDlg(QWidget *parent) : QDialog(parent), ret(true) {
	verticalLayout = new QVBoxLayout(this);
	verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setSpacing(1);
	verticalLayout->setContentsMargins(2, 2, 2, 2);

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(1);
	sizePolicy.setVerticalStretch(1);

	text = new QLineEdit(this);
	text->setSizePolicy(sizePolicy);
	text->installEventFilter(this);

	setSizeGripEnabled(true);
	setWindowModality(Qt::WindowModal);
}

void SelectInfoDlg::keyPressEvent( QKeyEvent *k ) {
	switch ( k->key() )    {
	case Qt::Key_Enter:
	case Qt::Key_Return:
		accept();
		break;
	case Qt::Key_F4:
	case Qt::Key_Escape:
		ret = false;
		reject();
		break;
	default:
		QWidget::keyPressEvent(k);
		break;
	};
}

bool SelectInfoDlg::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent *>(event);
		xmprintf(8, "SelectNameDlg::eventFilter: key %s (%d) \n", k->text().toStdString().c_str(), k->key());
		switch ( k->key() )    {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			ret = true;
			xmprintf(8, "SelectNameDlg::eventFilter: ENTER was pressed ! \n");
			event->ignore();
			accept();
			return true;
			break;
		default:
			 return QObject::eventFilter(obj, event);
		};
        
    } else {
        return QObject::eventFilter(obj, event);
    }
	return false;
}


SelectNameDlg::SelectNameDlg(QWidget *parent, const char* name)  : SelectInfoDlg(parent)  {
	setObjectName(QString::fromUtf8("SelectNameDlg"));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(1);
	sizePolicy.setVerticalStretch(1);
	
	verticalLayout->addWidget(text);

	if (name != 0) {
		text->setText(QString::fromUtf8(name));
	}
	resize(text->size());
}


SelectMarkerParamsDlg::SelectMarkerParamsDlg(QWidget *parent, const char* name)  : SelectInfoDlg(parent)   {
	setObjectName(QString::fromUtf8("SelectMarkerParamsDlg"));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(1);
	sizePolicy.setVerticalStretch(1);

	verticalLayout->addWidget(text);

	cbDirection = new QComboBox(this);
	cbDirection->addItem("bottom right");
	cbDirection->addItem("bottom left");
	cbDirection->addItem("top left");
	cbDirection->addItem("top right");
	verticalLayout->addWidget(cbDirection);
	cbDirection->setEditable(false);
	cbDirection->setCurrentIndex(qwSettings.direction);

	QFrame* f = new QFrame(this);
	f->setSizePolicy(sizePolicy);
	QHBoxLayout* hLayout = new QHBoxLayout(f);

	cpb = new QPushButton("&Color", this);
	cpb->setSizePolicy(sizePolicy);
	QColor co = qwSettings.markerColor();

	QPalette pal = cpb->palette();
	pal.setColor(QPalette::Button, co);
	selectedColor = co;
	cpb->setAutoFillBackground(true);
	cpb->setPalette(pal);
	cpb->update();

	hLayout->addWidget(cpb);

	okpb = new QPushButton("&OK", this);
	okpb->setSizePolicy(sizePolicy);
	hLayout->addWidget(okpb);
	//verticalLayout->addWidget(cpb);
	verticalLayout->addWidget(f);

	connect(cpb, &QPushButton::clicked, this, &SelectMarkerParamsDlg::onColor);
	connect(okpb, &QPushButton::clicked, this, &SelectMarkerParamsDlg::onOK);
	connect(cbDirection,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,  &SelectMarkerParamsDlg::directionChanged);

	if (name != 0) {
		text->setText(QString::fromUtf8(name));
	}

	//QSize fs = frame->size();
	//fs *= 1.5;
	//resize(fs);
}

void SelectMarkerParamsDlg::onColor() {
	QColor co = qwSettings.markerColor();
	selectedColor = co;
	QColor color = QColorDialog::getColor(co, this, "marker color");
	if (!color.isValid()) {
		xmprintf(5, "SelectMarkerParamsDlg::onColor(): color not valid \n");
		return;
	}
	if (color == co) {
		xmprintf(5, "SelectMarkerParamsDlg::onColor(): color is the same \n");
		return;
	}

	QPalette pal = cpb->palette();
	pal.setColor(QPalette::Button, color);
	cpb->setAutoFillBackground(true);
	cpb->setPalette(pal);
	cpb->update();

	qwSettings.aMarkerColor_R = color.red();
	qwSettings.aMarkerColor_G = color.green();
	qwSettings.aMarkerColor_B = color.blue();

	int test = qwSettings.qwSave();
	selectedColor = color;
}
void SelectMarkerParamsDlg::onOK() {
	accept();
}
void SelectMarkerParamsDlg::directionChanged(int dir) {
	if (dir != qwSettings.direction) {
		qwSettings.direction = dir;
		qwSettings.qwSave();
	}
}

void ArrowSymbol::asInit(double angle, int size) {
	QPen pen( Qt::black, 0 );
	pen.setJoinStyle( Qt::MiterJoin );

	setPen( pen );

	QColor co = qwSettings.markerColor();

	setBrush( co );

	QPainterPath path;
	path.moveTo( 0, 12 );
	path.lineTo( 0, 7 );
	path.lineTo( -3, 7 );
	path.lineTo( 0, 0 );
	path.lineTo( 3, 7 );
	path.lineTo( 0, 7 );

	QPen pen2( Qt::black, 2);
	setPen( pen2 );
	path.lineTo( 0, 12 );

	QTransform transform;
	transform.rotate(angle);
	path = transform.map( path );

	setPath( path );
	setPinPoint( QPointF( 0, 0 ) );

	setSize( size, size + 4);
}

ArrowSymbol::ArrowSymbol()    {
	asInit(-30.0, 14);
}

ArrowSymbol::ArrowSymbol(double angle, int size) {
	asInit(angle, size);
}

Figure2::Figure2(const std::string& key_, XQPlots* pf_, QWidget * parent, unsigned int flags_) : JustAplot(key_, pf_, parent, jQWT) {
	mouseMode = 0;
	flags = flags_;
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
	lastXselected = 0.0; lastYselected = 0.0;  pointWasSelected = false;
}

Figure2::~Figure2() {
	removeLines();
	//emit exiting(key);

	//if (panner != 0) { delete panner; panner = 0; }
	//if (picker != 0) { delete picker; picker = 0; }
	//if (zoomer != 0) { delete zoomer; zoomer = 0; }
}

void Figure2::ontb1(bool checked ) {  //   picker
	if (tbModeChanging) return;
	//picker->setEnabled(checked);
	//pf->setAllMarkersVisible(checked);
	
	

	//if (checked) {
	//	mode = 1;
	//} else {
	//	mode = 0;
	//}
	mouseMode = 1;

	setTBState(); 
	xmprintf(5, "Figure2::ontb1 mouseMode = %d \n", mouseMode);
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

void updateBounds(double& t1, double& t2, double t, bool& corrected1, bool& corrected2) {
	if (t < t1) {
		t1 = t;
		corrected1 = true;
	} else {
		if (t > t2) {
			t2 = t;
			corrected2 = true;
		}
	}
}



void Figure2::onClip(bool checked) {
	//  left and right points:
	clipperHost = true;
	QwtScaleMap smX = plot1->canvasMap(QwtPlot::xBottom);
	double  x1 = smX.s1();
	double  x2 = smX.s2();
	if (x1 > x2) {
		double tmp = x1; 
		x1 = x2;
		x2 = tmp;
	}
	QwtScaleMap smY = plot1->canvasMap(QwtPlot::yLeft);
	double  y1 = smY.s1();
	double  y2 = smY.s2();
	if (y1 > y2) {
		double tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	double tMin = BIGNUMBER;
	double tMax = -BIGNUMBER;
	bool minCorrected = false;
	bool maxCorrected = false;

	for (auto a : lines) {
		LineItemInfo* i = a->info;
		if ((i->mode == 3) && (i->time != 0)) {
			for (int k = 0; k < i->size; k++) {
				if (i->x[k] < x1) continue;
				if (i->x[k] > x2) continue;
				if (i->y[k] < y1) continue;
				if (i->y[k] > y2) continue;
				updateBounds(tMin, tMax, i->time[k], minCorrected, maxCorrected);
			}
		} else { 
			updateBounds(tMin, tMax, x1, minCorrected, maxCorrected);
			updateBounds(tMin, tMax, x2, minCorrected, maxCorrected);
		}

	}
	if (minCorrected && maxCorrected) {
		pf->clipAll(tMin, tMax, clipGroup);
	}
}

void Figure2::ontb2(bool checked ) { //   panner
	if (tbModeChanging) return;
	//panner->setEnabled(checked);
	//if (checked) {
//		mode = 2; 
	

	//} else {
	//	mode = 0;
	//}
	mouseMode = 2;
	setTBState();
	xmprintf(5, "Figure2::ontb2 mouseMode = %d \n", mouseMode);
}
void Figure2::ontb3(bool checked ) {  //  zoomer
	if (tbModeChanging) return;
	//zoomer->setEnabled(checked);
	//zoom(0);
	//if (checked) mode = 3; else mode = 0;
	mouseMode = 3;
	setTBState();
	xmprintf(5, "Figure2::ontb3 mouseMode = %d \n", mouseMode);
}

void Figure2::setTBState() {
	if (tbModeChanging) return;
	tbModeChanging = true;
	switch(mouseMode) {
	case 0: 
		tb1->setChecked(false);  tb2->setChecked(false);  tb3->setChecked(false);  
		picker->setEnabled(false);  panner->setEnabled(false);  zoomer->setEnabled(false); 
		break;
	case 1: 
		tb1->setChecked(true);   tb2->setChecked(false);  tb3->setChecked(false);  
		picker->setEnabled(true);  panner->setEnabled(false);  zoomer->setEnabled(false); 
		pf->setAllMarkersVisible(true);
		break;
	case 2: 
		tb1->setChecked(false);  tb2->setChecked(true);   tb3->setChecked(false);
		picker->setEnabled(false);  panner->setEnabled(true);  zoomer->setEnabled(false); 
		plot1->setCursor(Qt::OpenHandCursor);
		break;
	case 3: 
		tb1->setChecked(false);  tb2->setChecked(false);  tb3->setChecked(true); 
		picker->setEnabled(false);  panner->setEnabled(false);  zoomer->setEnabled(true); 
		break;
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
				case 'a': color = Qt::gray;    break;
				case 'A': color = Qt::darkGray; break;
				case 'h': color = Qt::lightGray; break;
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
			case 'e': sym->setStyle(QwtSymbol::Ellipse);  break; // Ellipse or circle.
			case 'r': sym->setStyle(QwtSymbol::Rect);  break; // Rectangle.
			case 'd': sym->setStyle(QwtSymbol::Diamond);  break;
			case 't': sym->setStyle(QwtSymbol::Triangle);  break; // Triangle pointing upwards.
			case 'T': sym->setStyle(QwtSymbol::DTriangle);  break; // Triangle pointing downwards.
			case 'L': sym->setStyle(QwtSymbol::LTriangle);  break; // Triangle pointing left.
			case 'R': sym->setStyle(QwtSymbol::RTriangle);  break; // Triangle pointing right.
			case 'x': sym->setStyle(QwtSymbol::Cross);  break; // Cross (+)
			case 's': sym->setStyle(QwtSymbol::Star1);  break; // X combined with +.
			case 'q': sym->setStyle(QwtSymbol::Star2);  break; // Six-pointed star.
			case 'w': sym->setStyle(QwtSymbol::XCross);  break; // Diagonal cross (X)
			case 'u': sym->setStyle(QwtSymbol::UTriangle);  break; // Triangle pointing upwards.
			case 'h': sym->setStyle(QwtSymbol::Hexagon);  break; // Hexagon.
			case 'a': sym->setStyle(QwtSymbol::HLine);  break; // Horizontal line.
			case 'b': sym->setStyle(QwtSymbol::VLine);  break; // Vertical line.
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

	if (line->mode == 3) {
		maybeTopView = true;
	}
}

void Figure2::removeLine(LineItemInfo* line) {
	const bool doReplot = plot1->autoReplot();
	plot1->setAutoReplot(false);
	
	//  remove from 'lines'
	std::list<FigureItem*>::iterator it;
	for (it = lines.begin(); it != lines.end(); it++) {
		FigureItem* i = *it;
		if (i->info == line) {

			//   delete and detach QWT line
			i->line->setData(NULL);
			i->line->detach();
			delete i->line;  //  delete QwtPlotCurve

			// delete FigureItem and its marker
			delete i;

			lines.erase(it);
			break;
		}
	}

	JustAplot::removeLine(line);  //   delete 'line' itself

	plot1->updateAxes();
	//zoomer->setZoomBase(true);
	plot1->setAutoReplot(doReplot);
	plot1->replot();
	zoomer->setZoomBase(false);
}

void Figure2::changeLine(LineItemInfo* line, double* x, double* y, double* z, double* time, int size) {
	if (size == line->size) {
	}	else {
		if (line->x != 0) { 
			delete[] line->x; 
			line->x = 0;
		}
		if (x != 0) {
			line->x = new double[size];
		}

		if (line->y != 0) {
			delete[] line->y;
			line->y = 0;
		}
		if (y != 0) {
			line->y = new double[size];
		}

		if (line->z != 0) {
			delete[] line->z;
			line->z = 0;
		}
		if (z != 0) {
			line->z = new double[size];
		}

		if (line->time != 0) {
			delete[] line->time;
			line->time = 0;
		}
		if (time != 0) {
			line->time = new double[size];
		}

		line->size = size;
	}

	if (x != 0) {
		memcpy(line->x, x, size * sizeof(double));
	}
	if (y != 0) {
		memcpy(line->y, y, size * sizeof(double));
	}
	if (z != 0) {
		memcpy(line->z, z, size * sizeof(double));
	}
	if (time != 0) {
		memcpy(line->time, time, size * sizeof(double));
	}

	plot1->replot();
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

	plot1 = new FSPlot(this, flags);
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
		delete cf->line; // this was QwtPlotCurve

		delete cf;
		
		xi++;
	}
	lines.clear();
	remove_lines();

	if (!vmList.empty()) {//  remove all the vertical markers
		for (VLineMarker* a : vmList) {
			a->detach();
			delete a;
		}
		vmList.clear();
	}
	if (!amList.empty()) {//  remove all the other markers
		for (AMarker* a : amList) {
			a->detach();
			delete a;
		}
		amList.clear();
	}
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
	int ws = dlg.getWindowSize();
	char stmp[16];
	snprintf(stmp, 16, "%u", ws);

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
	// do we need to see corrected title after mouse was released ? setWindowTitle(sTitle.c_str());
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

void Figure2::keyPressEvent( QKeyEvent *k ) {
	switch ( k->key() )    {
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right:
		QWidget::keyPressEvent(k);
		break;
	case Qt::Key_V: //  add vertical marker
		if (k->modifiers() & Qt::ShiftModifier) {
			xmprintf(9, "Shift + V was pressed!\n");
			addGlobalVMarker();
		} else {
			xmprintf(9, "V was pressed!\n");
			addVMarker();
		}
		break;
	case Qt::Key_A:  //  add arrow marker
		xmprintf(9, "A was pressed!\n");
		addAMarker();
		break;
	//case Qt::Key_B:  //  add arrow marker (top left pos)
	//	xmprintf(9, "B was pressed!\n");
	//	addAMarker(AMarker::amTopLeft);
	//	break;
	case Qt::Key_M: //  switch to marker mode
		mouseMode = 1;
		setTBState(); 
		break;
	case Qt::Key_P: //  switch to pan mode
		mouseMode = 2;
		setTBState(); 
		break;
	case Qt::Key_Z: //  switch to zoom mode
		mouseMode = 3;
		setTBState(); 
		break;
	default:
			QWidget::keyPressEvent(k);
			break;
	};
	
	return;
}

int Figure2::markerTest(int type, int& mid, std::string& label, QColor& color) {
	int ret = 0;
	mid = 0;
	if (mouseMode != 1) {
		xmprintf(3, "Figure2::markerTest(): mouseMode = %d \n", mouseMode);
		return 0;
	}
	if (!pointWasSelected) {
		xmprintf(3, "Figure2::markerTest(): point was not selected \n");
		return 0;
	}
	
	xmprintf(3, "Figure2::markerTest(): ..... \n");
	bool haveItAlready = false;
	QwtScaleMap smY = plot1->canvasMap(QwtPlot::yLeft);
	QwtScaleMap smX = plot1->canvasMap(QwtPlot::xBottom);
	double dxS = smX.sDist();
	double dyS = smY.sDist();

	double dx = dxS / 256.0;
	double dy = dyS / 100.0;

	switch (type) {
	case 1: //  vertical
		for (VLineMarker* a : vmList) {
			if (fabs(lastXselected - a->t) < dx) {
				haveItAlready = true;
				mid = a->id;
				break;
			}
		}
		break;
	case 2:  // arrow
		for (AMarker* a : amList) {
			if ((fabs(lastXselected - a->x) < dx) && (fabs(lastYselected - a->y) < dy)) {
				haveItAlready = true;
				mid = a->id;
				break;
			}
		}
		break;
	};
	if (haveItAlready) {
		return 2;
	}
	char tmp[64];
	switch(type) {
	case 1:
		snprintf(tmp, 64, "%.2f", lastXselected);
		{
			SelectNameDlg dlg(this, tmp);
			dlg.exec();
			if (!dlg.ret) {
				xmprintf(3, "Figure2::markerTest(): rejected \n");
				return 0;
			}
			label = dlg.text->text().toStdString();
		}
		break;
	case 2: 
		snprintf(tmp, 64, "[%.2f, %.2f]", lastXselected, lastYselected);
		{
			SelectMarkerParamsDlg dlg(this, tmp);
			dlg.exec();
			if (!dlg.ret) {
				xmprintf(3, "Figure2::markerTest(): rejected \n");
				return 0;
			}
			label = dlg.text->text().toStdString();
			color = dlg.selectedColor;
		}
		break;
	};


	return 1;
}

void Figure2::addGlobalVMarker() {
	int mid = 0;
	std::string label;
	QColor color;
	int  test = markerTest(1, mid, label, color);
	switch(test) {
	case 1:  
		markerID++;
		pf->addVMarkerEverywhere(lastXselected, label.c_str(), markerID, this);
		break;
	case 2:
		pf->removeVMarkerEverywhere(mid);
		break;
	};
	xmprintf(3, "Figure2::addGlobalVMarker(): OK \n");
}

void Figure2::addVMarker() {
	int mid = 0;
	std::string label;
	QColor color;
	int  test = markerTest(1, mid, label, color);
	switch(test) {
	case 1:  
		markerID++;
		addVMarker(lastXselected, label.c_str(), markerID);
		break;
	case 2:
		removeVMarker(mid);
		break;
	};
	xmprintf(3, "Figure2::addVMarker(): OK \n");
}

void Figure2::addVMarker(double t, const char* label, int id_) {
	VLineMarker* vm = new VLineMarker(label, t, id_);
	vm->attach(plot1);
	vmList.push_back(vm);
	replot();
}

void Figure2::removeVMarker(int id_) {
	std::list<VLineMarker*>::iterator it = vmList.begin();
	while (it != vmList.end()) {
		VLineMarker* a = *it;
		int test = a->id;
		if (a->id == id_) {
			a->detach();
			//vmList.remove(a);
			delete a;
			it = vmList.erase(it);
			//  do not break here
		} else {
			++it;
		}
	}
}

void Figure2::addAMarker() {
	int mid = 0;
	std::string label;
	QColor color;
	AMarker* am;
	int  test = markerTest(2, mid, label, color);
	switch(test) {
	case 1:  
		markerID++;
		am = new AMarker(label.c_str(), lastXselected, lastYselected, color, 
				static_cast<AMarker::AMPos>(qwSettings.direction), markerID);
		amList.push_back(am);
		am->attach(plot1);
		break;
	case 2:
		{
			std::list<AMarker*>::iterator it = amList.begin();
			while (it != amList.end()) {
				AMarker* a = *it;
				if (a->id == mid) {
					a->detach();
					delete a;
					it = amList.erase(it);
				} else {
					++it;
				}
			}
		}
		break;
	};
	replot();
	xmprintf(3, "Figure2::addAMarker(): OK \n");
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
	
   
	lastXselected = mfi->info->x[minIndex];
	lastYselected = mfi->info->y[minIndex];
	pointWasSelected = true;
	//std::ostringstream s; 
	//s << xxm << ", " << yym << ", (" << mfi->info->legend << ")";
	char s[256];
	char s0[128];
	int dn = qwSettings.pickerDigitsNumber;  // how many digits to draw for each number
	if (mfi->info->mode == 3) {
		snprintf(s0, 128, "[%%.%df, %%.%df] t=%%.3f (%%s), index=%%lld", dn, dn);
		snprintf(s, 256, s0,
			lastXselected, lastYselected, t, mfi->info->legend.c_str(), minIndex);
	} else {
		snprintf(s0, 128, "%%.%df, %%.%df (%%s), index=%%lld", dn, dn);
		snprintf(s, 256, s0,
			lastXselected, lastYselected, mfi->info->legend.c_str(), minIndex);
	}

	setWindowTitle(s);
	
	if(ok) {
	   //pf->drawAllMarkers(t);
		pf->drawAllMarkers2(iKey, mfi->info->id, minIndex, x, y, lastXselected, lastYselected, t, mfi->info->legend);
	}

	//xmprintf(8, "Figure2::onPickerSigna: time = %f  \n", t);
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
	JustAplot::drawMarker(t);   //  update markers for all the 'linesInfo'

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
		if (!i->lCheck()) {
			xmprintf(1, "ERROR: Figure2::onClip bad LineItemInfo \n");
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

//void Figure2::setAxesEqual() {
//	zoomer->keepEqual(true);
//}

SLDialog::SLDialog(std::list<FigureItem*> lines, QWidget *parent) : QDialog(parent) {
	ui.setupUi(this);
	QVBoxLayout* vbl = new QVBoxLayout(ui.saFrame);
	char stmp[16];


	unsigned int ws = 1;
	do {
		snprintf(stmp, 16, "%u", ws);
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


