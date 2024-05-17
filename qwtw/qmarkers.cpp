
#include "qmarkers.h"

#include <qfont.h>
#include <QButtonGroup>
#include <QDialog>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <QList>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <QColorDialog>
#include "qpainterpath.h"
#include <qwt_text.h>

#include "settings.h"

int xmprintf(int level, const char* _Format, ...);

int markerID = 1;

VLineMarker::VLineMarker(const char* text, double time, int id_) : t(time), QWMarker(id_) {
	QwtText label(text);
	label.setFont(QFont("Consolas", 12, QFont::Bold));
	label.setBackgroundBrush(QBrush(QColor(250, 250, 250)));
	QPen pen(Qt::black, 1);
	label.setBorderPen(pen);

	setLabel(label);
	setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
	setLabelOrientation(Qt::Vertical);
	setLineStyle(QwtPlotMarker::VLine);
	setLinePen(Qt::black, 2, Qt::DashDotLine);
	setXValue(time);
}

AMarker::AMarker(const char* text, double x_, double y_, const QColor& color_, int id_) : QWMarker(id_) {
	amInit(text, x_, y_, color_, amBottomRight);
}
AMarker::AMarker(const char* text, double x_, double y_, const QColor& color_, AMarker::AMPos pos_, int id_) : QWMarker(id_) {
	amInit(text, x_, y_, color_, pos_);
}
void AMarker::amInit(const char* text, double x_, double y_, const QColor& color_, AMPos pos_) {
	x = x_;
	y = y_;
	color = color_;
	pos = pos_;

	QwtText label(text);
	label.setFont(QFont("Consolas", 12, QFont::Bold));

	//QColor co = qwSettings.markerColor();

	label.setColor(color);
	label.setBackgroundBrush(QBrush(QColor(250, 250, 250)));
	QPen pen(color, 1);
	label.setBorderPen(pen);

	setRenderHint(QwtPlotItem::RenderAntialiased, true);
	setItemAttribute(QwtPlotItem::Legend, false);
	setLabel(label);
	ArrowSymbol* as;
	switch (pos) {

	case amTopLeft:
		as = new ArrowSymbol(150.0, 14);
		setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
		break;

	case amTopRight:
		as = new ArrowSymbol(-150.0, 14);
		setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
		break;

	case amBottomLeft:
		as = new ArrowSymbol(30.0, 14);
		setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
		break;

	case amBottomRight:
	default:
		as = new ArrowSymbol();
		setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
		break;
	};
	setSymbol(as);
	setValue(QPointF(x, y));
}



SelectInfoDlg::SelectInfoDlg(QWidget* parent) : QDialog(parent), ret(true) {
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

void SelectInfoDlg::keyPressEvent(QKeyEvent* k) {
	switch (k->key()) {
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

bool SelectInfoDlg::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* k = static_cast<QKeyEvent*>(event);
		xmprintf(8, "SelectNameDlg::eventFilter: key %s (%d) \n", k->text().toStdString().c_str(), k->key());
		switch (k->key()) {
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


SelectNameDlg::SelectNameDlg(QWidget* parent, const char* name) : SelectInfoDlg(parent) {
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


SelectMarkerParamsDlg::SelectMarkerParamsDlg(QWidget* parent, const char* name) : SelectInfoDlg(parent) {
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
	connect(cbDirection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SelectMarkerParamsDlg::directionChanged);

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
	QPen pen(Qt::black, 0);
	pen.setJoinStyle(Qt::MiterJoin);

	setPen(pen);

	QColor co = qwSettings.markerColor();

	setBrush(co);

	QPainterPath path;
	path.moveTo(0, 12);
	path.lineTo(0, 7);
	path.lineTo(-3, 7);
	path.lineTo(0, 0);
	path.lineTo(3, 7);
	path.lineTo(0, 7);

	QPen pen2(Qt::black, 2);
	setPen(pen2);
	path.lineTo(0, 12);

	QTransform transform;
	transform.rotate(angle);
	path = transform.map(path);

	setPath(path);
	setPinPoint(QPointF(0, 0));

	setSize(size, size + 4);
}

ArrowSymbol::ArrowSymbol() {
	asInit(-30.0, 14);
}

ArrowSymbol::ArrowSymbol(double angle, int size) {
	asInit(angle, size);
}




