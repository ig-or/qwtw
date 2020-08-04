

#include "qt-3d.h"

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QComboBox>
#include <QCheckBox>
#include <QFontComboBox>
#include <QPushButton>

#include "xstdef.h"
#include "sfigure.h"
#include "xmcoords.h"
#include "xmutils.h"


Q3DView::Q3DView(const std::string& key_, XQPlots* pf_, QWidget *parent) : JustAplot(key_, pf_, parent, 3) {
	//data = 0;
//	data.resize(0);
}

Q3DView::~Q3DView() {
	emit exiting(key);
}

int Q3DView::q3Init() {
	selecting = false;  activeSeries = 0;
	setWindowTitle("3D test !");
	using namespace QtDataVisualization;
	scatter = new Q3DScatter();
	QWidget *container = QWidget::createWindowContainer(scatter);
	//scatter->setFlags(scatter->flags() ^ Qt::FramelessWindowHint);
	QHBoxLayout *hLayout = new QHBoxLayout(this);
	QVBoxLayout *vLayout = new QVBoxLayout();
	hLayout->addWidget(container, 1);
	hLayout->addLayout(vLayout);


	scatter->activeTheme()->setType(Q3DTheme::ThemeEbony);
	QFont font = scatter->activeTheme()->font();
	font.setPointSize(14);
	scatter->activeTheme()->setFont(font);
	scatter->setShadowQuality(QAbstract3DGraph::ShadowQualitySoftLow);
	scatter->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
	scatter->scene()->activeCamera()->setXRotation(90.);

/*	QScatter3DSeries *series = new QScatter3DSeries;
	QScatterDataArray data;
	data << QVector3D(0.5f, 0.5f, 0.5f) << QVector3D(-0.3f, -0.5f, -0.4f) << QVector3D(0.0f, -0.3f, 0.2f);
	series->dataProxy()->addItems(data);
	scatter->addSeries(series);
*/
	//scatter->show();
	setSizeGripEnabled(true);
	hLayout->setSpacing(1);
	//hLayout->
	scatter->setMinimumHeight(250);
	setMinimumHeight(300);  setMinimumWidth(300);
	//setSize


	//   add appearance control:
	QComboBox *themeList = new QComboBox(this);
	themeList->addItem(QStringLiteral("Qt"));
	themeList->addItem(QStringLiteral("Primary Colors"));
	themeList->addItem(QStringLiteral("Digia"));
	themeList->addItem(QStringLiteral("Stone Moss"));
	themeList->addItem(QStringLiteral("Army Blue"));
	themeList->addItem(QStringLiteral("Retro"));
	themeList->addItem(QStringLiteral("Ebony"));
	themeList->addItem(QStringLiteral("Isabelle"));
	themeList->setCurrentIndex(6);

	QComboBox *itemStyleList = new QComboBox(this);
	itemStyleList->addItem(QStringLiteral("Sphere"), int(QAbstract3DSeries::MeshSphere));
	itemStyleList->addItem(QStringLiteral("Cube"), int(QAbstract3DSeries::MeshCube));
	itemStyleList->addItem(QStringLiteral("Minimal"), int(QAbstract3DSeries::MeshMinimal));
	itemStyleList->addItem(QStringLiteral("Point"), int(QAbstract3DSeries::MeshPoint));
	itemStyleList->setCurrentIndex(0);

	QComboBox *shadowQuality = new QComboBox(this);
	shadowQuality->addItem(QStringLiteral("None"));
	shadowQuality->addItem(QStringLiteral("Low"));
	shadowQuality->addItem(QStringLiteral("Medium"));
	shadowQuality->addItem(QStringLiteral("High"));
	shadowQuality->addItem(QStringLiteral("Low Soft"));
	shadowQuality->addItem(QStringLiteral("Medium Soft"));
	shadowQuality->addItem(QStringLiteral("High Soft"));
	shadowQuality->setCurrentIndex(4);

	vLayout->addWidget(themeList);
	vLayout->addWidget(itemStyleList);
	vLayout->addWidget(shadowQuality);

	QCheckBox *backgroundCheckBox = new QCheckBox(this);
	backgroundCheckBox->setText(QStringLiteral("Show background"));
	backgroundCheckBox->setChecked(true);

	QCheckBox *gridCheckBox = new QCheckBox(this);
	gridCheckBox->setText(QStringLiteral("Show grid"));
	gridCheckBox->setChecked(true);

	QPushButton *cameraButton = new QPushButton(this);
	cameraButton->setText(QStringLiteral("Change camera preset"));

//	QFontComboBox *fontList = new QFontComboBox(this);
	//fontList->setCurrentFont(QFont("Arial"));
	vLayout->addWidget(cameraButton, 0, Qt::AlignTop);

	vLayout->addWidget(backgroundCheckBox);
	vLayout->addWidget(gridCheckBox);
	//vLayout->addWidget(fontList);


	bool test;

	QObject::connect(cameraButton, &QPushButton::clicked, this,
		&Q3DView::changePresetCamera);
	//QObject::connect(labelButton, &QPushButton::clicked, modifier,
	//	&ScatterDataModifier::changeLabelStyle);
	//QObject::connect(itemCountButton, &QPushButton::clicked, modifier,
	//	&ScatterDataModifier::toggleItemCount);

	test = QObject::connect(backgroundCheckBox, &QCheckBox::stateChanged, this,
		&Q3DView::setBackgroundEnabled);
	test = QObject::connect(gridCheckBox, &QCheckBox::stateChanged, this,
		&Q3DView::setGridEnabled);
	//QObject::connect(smoothCheckBox, &QCheckBox::stateChanged, this,
	//	&Q3DView::setSmoothDots);

	test = QObject::connect(this, &Q3DView::backgroundEnabledChanged,
		backgroundCheckBox, &QCheckBox::setChecked);
	test = QObject::connect(this, &Q3DView::gridEnabledChanged,
		gridCheckBox, &QCheckBox::setChecked);
	test = QObject::connect(itemStyleList, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeStyle(int)));

	test = QObject::connect(themeList, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeTheme(int)));

	test = QObject::connect(shadowQuality, SIGNAL(currentIndexChanged(int)), this,
		SLOT(changeShadowQuality(int)));

	test = QObject::connect(this, &Q3DView::shadowQualityChanged, shadowQuality,
		&QComboBox::setCurrentIndex);
	test = QObject::connect(scatter, &Q3DScatter::shadowQualityChanged, this,
		&Q3DView::shadowQualityUpdatedByVisual);

	//QObject::connect(fontList, &QFontComboBox::currentFontChanged, this,
	//	&Q3DView::changeFont);

	//QObject::connect(this, &Q3DView::fontChanged, fontList, &QFontComboBox::setCurrentFont);
	scatter->setCursor(Qt::ArrowCursor);

	return 0;
}

void Q3DView::changeStyle(int style) {
	using namespace QtDataVisualization;

	QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
	if (comboBox) {
		m_style = QAbstract3DSeries::Mesh(comboBox->itemData(style).toInt());
		if (scatter->seriesList().size())
			scatter->seriesList().at(0)->setMesh((QAbstract3DSeries::Mesh)(m_style));
	}
}

void Q3DView::setSmoothDots(int smooth) {
	using namespace QtDataVisualization;

	m_smooth = bool(smooth);
	QScatter3DSeries *series = scatter->seriesList().at(0);
	series->setMeshSmooth(m_smooth);
}

void Q3DView::changeTheme(int theme) {
	using namespace QtDataVisualization;

	Q3DTheme *currentTheme = scatter->activeTheme();
	currentTheme->setType(Q3DTheme::Theme(theme));
	emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
	emit gridEnabledChanged(currentTheme->isGridEnabled());
	emit fontChanged(currentTheme->font());
}

void Q3DView::changePresetCamera() {
	using namespace QtDataVisualization;

	static int preset = Q3DCamera::CameraPresetFrontLow;

	scatter->scene()->activeCamera()->setCameraPreset((Q3DCamera::CameraPreset)preset);

	if (++preset > Q3DCamera::CameraPresetDirectlyBelow)
		preset = Q3DCamera::CameraPresetFrontLow;
}

void Q3DView::changeLabelStyle() {
	using namespace QtDataVisualization;

	scatter->activeTheme()->setLabelBackgroundEnabled(!scatter->activeTheme()->isLabelBackgroundEnabled());
}

void Q3DView::changeFont(const QFont &font) {
	using namespace QtDataVisualization;

	QFont newFont = font;
	newFont.setPointSizeF(m_fontSize);
	scatter->activeTheme()->setFont(newFont);
}

void Q3DView::shadowQualityUpdatedByVisual(int sq) {
	using namespace QtDataVisualization;

	int quality = sq;
	emit shadowQualityChanged(quality); // connected to a checkbox in main.cpp
}

void Q3DView::changeShadowQuality(int quality) {
	using namespace QtDataVisualization;

	QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
	scatter->setShadowQuality(sq);
}

void Q3DView::setBackgroundEnabled(int enabled) {
	using namespace QtDataVisualization;

	scatter->activeTheme()->setBackgroundEnabled((bool)enabled);
}

void Q3DView::setGridEnabled(int enabled) {
	using namespace QtDataVisualization;

	scatter->activeTheme()->setGridEnabled((bool)enabled);
}

void Q3DView::onHome() {
	//mw->onHome();
}

void Q3DView::selectedItemChanged(int index) {
	using namespace QtDataVisualization;
	if (selecting) {
		return;
	}
	if (index == -1) {
		return;
	}
	selecting = true;
	QObject* obj = const_cast<QObject*>(sender());
	if (obj == 0) {
		selecting = false;
		return;
	}

	//QScatter3DSeries* series = qobject_cast<QScatter3DSeries*>obj;
	QScatter3DSeries* series = (QScatter3DSeries*)(obj);

	if (series == 0) {
		selecting = false;
		return;
	}
	std::map<QScatter3DSeries*, LineItemInfo* >::iterator it = s.find(series);
	if (it == s.end()) {
		selecting = false;
		return;
	}
	LineItemInfo* line = it->second;
	if (line->size == 0) {
		selecting = false;
		return;
	}
	if (index >= line->size) {
		selecting = false;
		return;
	}

	emit onSelection(key);

//	char s[256];
//	sprintf(s, "[%.6f, %.6f, %.6f] (%d points)",  //  NOT %lld !!!!!!
//		line->x[index], line->y[index], line->z[index], line->size);
//
	// setWindowTitle(s.str().c_str());
//	setWindowTitle(s);

	activeSeries = series;
	if (line->time != 0) {
		pf->drawAllMarkers(line->time[index]);
	}

	pf->setAllMarkersVisible(true);
	activeSeries = 0;
	selecting = false;
}

void Q3DView::drawMarker(double t) {
	using namespace QtDataVisualization;

	JustAplot::drawMarker(t);
	std::map<QScatter3DSeries*, LineItemInfo* >::iterator it = s.begin();
	while (it != s.end()) {
		int i = it->second->ma.index;
		if ((i >= 0) && (i < it->second->size)) {
			if ((selecting) /* && (it->first == activeSeries))*/) {

			} else {
				selecting = true;
				it->first->setSelectedItem(i);
				selecting = false;
			}
		}
		
		it++;
	}

}


void Q3DView::addLine(LineItemInfo* line) {
	JustAplot::addLine(line);
	mxat(line->z != 0);
	if (line->z == 0) {
		return;
	}
	using namespace QtDataVisualization;

	QScatterDataProxy* proxy = new QScatterDataProxy;
	QScatter3DSeries* series = new QScatter3DSeries(proxy);
	series->setItemLabelFormat(QStringLiteral("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel"));
	series->setMeshSmooth(true);
	QScatterDataArray *data = new QScatterDataArray;

	data->resize(line->size);
	for (int i = 0; i < line->size; i++) {
		(*data)[i].setPosition(QVector3D(line->x[i], line->y[i], line->z[i]));
	
	}
	series->dataProxy()->resetArray(data);

	QColor color = Qt::GlobalColor::darkGreen;

	if (line->style == std::string()) {
		//
	} else {
		int sn = line->style.size();

		if (sn > 0) { //    last is always color:
					  //  set color:
			switch (line->style[sn - 1]) {
			case 'r':  color = Qt::GlobalColor::red; break;
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
		}
	}


	series->setBaseColor(color);

	scatter->addSeries(series);


	bool 	test = QObject::connect(series, SIGNAL(selectedItemChanged(int)), this,
		SLOT(selectedItemChanged(int)));
	s.insert(std::make_pair(series, line));
}

