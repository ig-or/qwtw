
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
#include <map>
#include <thread>
#include <mutex>
//#include <QMainWindow>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include "qwtypes.h"

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
//struct FigureItem;
#ifdef ENABLE_UDP_SYNC
class BCUdpClient;
class BCUdpServer;
#endif

/*
class PlotsInterfaceModel: public QStandardItemModel {
	public: 


};
*/
//typedef void (*printer_t)(int);

struct LineHandler {
	LineItemInfo* line;
	JustAplot* plot;
};

class /*qqwtw_API*/ XQPlots: public /*QMainWindow { //  */ QDialog {
	Q_OBJECT

public:
	bool markersAreVisible;
	XQPlots(QWidget * parent1 = 0);
	~XQPlots();

	/**    create a plot with a number, or raise up  already created plot  
		@param[in] n a plot number. There can be only one plot with this number in PlotFactory.
		@param[in] type plot type; '1' - QWT plot; '2' - map (top view); '3' - 3D plot
		@return pointer to the created plot
	*/
	JustAplot* figure(int n, JPType type = jQWT, unsigned int flags = 0);
	JustAplot* figure(std::string name_, JPType type = jQWT, unsigned int flags = 0);
	void title(const std::string& s);
	void footer(const std::string& s);
	void xlabel(const std::string& s);
	int service(int x);
	void ylabel(const std::string& s);
	void setClipGroup(int cg);
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
	int plot(double* x, double* y, int size, const char* name, const char* style, int lineWidth, int symSize,
	    double* time = 0);

	int plot(double* x, double* y, double* z, int size, const char* name, const char* style, int lineWidth, int symSize,
		double* time = 0);

	int removeLine(int key);
	int changeLine(int key, double* x, double* y, double* z, double* time, int size);

	void mesh(const MeshInfo& info);

#ifdef ENABLE_UDP_SYNC
	void enableCoordBroadcast(double* x, double* y, double* z, double* time, int size); 
	void disableCoordBroacast();
#endif



	/**
	@param[in] type type of marker; 1 - vertical line; 2 - point;
	*/
	void drawMarker(const std::string& key_, double X, double Y, int type = 1);

	/// <summary>
	///  callback from UDP
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="z"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	Q_INVOKABLE void drawAllMarkers1(int index, double x, double y, double z, double t);

	/// <summary>
	/// callback from plot picker (mouse click)
	/// </summary>
	/// <param name="figureID"> figure ID (iKey)  </param>
	/// <param name="lineID">    line ID (static.... not threadsafe)</param>
	/// <param name="index"> index of the selected  point </param>
	/// <param name="fx"> x plot coord</param>
	/// <param name="fy"></param>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="t"></param>
	/// <param name="legend"> legend of the selected line </param>
	/// <returns></returns>
	Q_INVOKABLE void drawAllMarkers2(int figureID, int lineID, int index, int fx, int fy, double x, double y, double t, const std::string& legend);

	Q_INVOKABLE void drawAllMarkers(double t);
	Q_INVOKABLE void addVMarkerEverywhere(double t, const char* label = 0, int id_ = 0);
	Q_INVOKABLE void removeVMarkerEverywhere(int id_);

	void clipAll(double t1, double t2, int clipGroup);
	void setUdpCallback(OnUdpCallback  cb);
	void setPickerCallback(OnPickerCallback cb);


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
	bool callbackWorking = false;
	void sendBroadcast(double x, double y, double z);
	
	///   this will send a 'picker info' over UDP 
	/// 	(to the local host and some port)
	void sendPickerInfo(const CBPickerInfo& cpi);
#endif
	JustAplot*	cf;	
	QWidget* parent;
	int currentClipGroup;
	/** marker - related stuff

	*/
	//int currentFigureMode; 
	bool currentImportanceMode;
	bool clearingAllFigures;
	void clearFigures();

private:
	//PlotsInterfaceModel pim;
	QStandardItemModel pim;
	/**
	@return 0 if cannot find
	*/
	JustAplot* getPlotByName(std::string s);
	void on3DMarker(double p[3]);
//	QStandardItem* jp2SI
	std::map<int, LineHandler> lines;
	OnUdpCallback onUdpCallback = 0;
	OnPickerCallback onPickerCallback = 0;

	/// special picker filtering things
	std::thread pFilterThread; ///< filtering thread handler
	std::mutex  pFilterMutex;  ///< for access cbi and haveNewPickerInfo
	CBPickerInfo cbi;	///< filtering info container
	bool pleaseStopFilterThread = false;
	bool haveNewPickerInfo = false; ///< new picker info to the filtering thread flag
	void pFilterThreadF(); ///< filtering thread function


private slots:
	void onFigureClosed(const std::string& key);
	void onSelection(const std::string& key);
	void onPicker(const std::string& key_, double X, double Y);
	void onTvItemClicked(QModelIndex mi);
	void onCloseAllPlots(bool checked);
	void onShowAllPlots(bool checked);
	void onShowAllSimple(bool checked);

	void onTest();
};





#endif



