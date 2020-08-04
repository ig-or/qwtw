
/**

	2D and 3D line views container.
	

	\file sfigure.h
	\author   Igor Sandler
	\date    Jul 2009
	\version 1.0
	
*/



#ifndef SFIGURE_H_FILE
#define SFIGURE_H_FILE

#include <QObject>
#include <QMap>
#include <QWidget>
#include <QDialog>
#include <QAbstractItemModel>
#include <QStandardItemModel>

#ifdef WIN32
#ifdef qqwtwEXPORTS	
#define qqwtw_API  Q_DECL_EXPORT // __declspec(dllexport)
#else
#define qqwtw_API   Q_DECL_IMPORT // __declspec(dllimport)
#endif
#else
#define qqwtw_API
#endif

class JustAplot;


#include "ui_mw.h"
struct LineItemInfo;
#ifdef ENABLE_UDP_SYNC
class BCUdpClient;
class BCUdpServer;
#endif


class PlotsInterfaceModel: public QStandardItemModel {
	public: 


};

class qqwtw_API XQPlots: public QDialog {
	Q_OBJECT

public:
	bool markersAreVisible;
	XQPlots(QWidget * parent1 = 0);
	~XQPlots();

	/**    create a plot with a number, or raise up  already created plot  
		@param[in] n a plot number. There can be only one plot with this number in PlotFactory.
		@param[in] type plot type; '1' - QWT plot; '2' - map (top view); '3' - 3D plot
	*/
	JustAplot* figure(int n, int type = 1);
	JustAplot* figure(std::string name_, int type = 1);
	void title(const std::string& s);
	void footer(const std::string& s);
	void xlabel(const std::string& s);
	void ylabel(const std::string& s);
	/** change marker mode for new plots

	*/
	void setmode(int mode_);
	void setAllMarkersVisible(bool visible);

	void setImportant(bool i);

	/**  close all plots.

	*/
	void clear();

	void showMainWindow();

	/**

	*/
	void plot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize,
	    double* time = 0);

	void plot(double* x, double* y, double* z, int size, const char* name, const char* style, int lineWidth, int symSize,
		double* time = 0);

#ifdef ENABLE_UDP_SYNC
	void enableCoordBroadcast(double* x, double* y, double* z, double* time, int size); 
	void disableCoordBroacast();
#endif



	/**
	@param[in] type type of marker; 1 - vertical line; 2 - point;
	*/
	void drawMarker(const std::string& key_, double X, double Y, int type = 1);

	Q_INVOKABLE void drawAllMarkers(double t);

	void clipAll(double t1, double t2);


	//void figure3(int n);
	void onExit();
	//void setEcefOrigin(double* origin);

signals:
	void selection(const std::string&);
	void picker(const std::string& key_, double X, double Y);
	void figureClosed(const std::string& key);

protected:
	Ui_topviewplotdlg    ui;
	typedef std::map<std::string, JustAplot*> FSet;
	FSet figures;
#ifdef ENABLE_UDP_SYNC
	LineItemInfo* broadCastInfo;
	BCUdpClient*	bc;
	BCUdpServer*	bServer;
	void sendBroadcast(double x, double y, double z);
#endif
	JustAplot*	cf;	
	QWidget* parent;
	/** marker - related stuff

	*/
	//int currentFigureMode; 
	bool currentImportanceMode;
	bool clearingAllFigures;
	void clearFigures();

private:
	//double ecefOrigin[3];
	PlotsInterfaceModel pim;
	/**
	@return 0 if cannot find
	*/
	JustAplot* getPlotByName(std::string s);
	void on3DMarker(double p[3]);
//	QStandardItem* jp2SI

private slots:
	void onFigureClosed(const std::string& key);
	void onSelection(const std::string& key);
	void onPicker(const std::string& key_, double X, double Y);
	void onTvItemClicked(QModelIndex mi);
	void onCloseAllPlots(bool checked);
	void onShowAllPlots(bool checked);
	void onShowAllSimple(bool checked);
};





#endif



