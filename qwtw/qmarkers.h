#pragma once

#include <QDialog>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

class QLineEdit;
class QComboBox;
class QVBoxLayout;

extern int markerID;

class QWMarker : public QwtPlotMarker {
public:
	QWMarker(int id_) : id(id_) {}
	int id;
};

class VLineMarker :public QWMarker {
public:
	double t;
	VLineMarker(const char* text, double time, int id_ = 0);
};

class AMarker : public QWMarker {
public:
	enum AMPos {
		amBottomRight = 0,
		amBottomLeft,
		amTopLeft,
		amTopRight
	};
	double x, y;
	AMPos pos;
	QColor color;
	AMarker(const char* text, double x_, double y_, const QColor& color_, int id_ = 0);
	AMarker(const char* text, double x_, double y_, const QColor& color_, AMarker::AMPos pos_, int id_ = 0);
private:
	void amInit(const char* text, double x_, double y_, const QColor& color_, AMPos pos_);
};


class ArrowSymbol : public QwtSymbol {
public:
	ArrowSymbol();
	ArrowSymbol(double angle, int size);
private:
	void asInit(double angle, int size);
};

class SelectInfoDlg : public QDialog {
	Q_OBJECT
public:
	SelectInfoDlg(QWidget* parent);
	QLineEdit* text;
	QComboBox* cbDirection;
	QVBoxLayout* verticalLayout;
	bool ret;
protected:
	bool eventFilter(QObject* obj, QEvent* event) override;
	void keyPressEvent(QKeyEvent* k);
};

class SelectNameDlg : public SelectInfoDlg {
	Q_OBJECT
public:
	SelectNameDlg(QWidget* parent = 0, const char* name = 0);
};

class SelectMarkerParamsDlg : public SelectInfoDlg {
	Q_OBJECT
public:
	SelectMarkerParamsDlg(QWidget* parent = 0, const char* name = 0);
	QPushButton* cpb;
	QPushButton* okpb;
	QColor selectedColor;

public slots:
	void onColor();
	void onOK();
	void directionChanged(int dir);

};


