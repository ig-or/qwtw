

#ifndef JUSTAPLOT_H_FILE
#define JUSTAPLOT_H_FILE

#include <QDialog>
#include <list>
#include "qwtypes.h"

class XQPlots;
struct LineItemInfo;


long long fcp(LineItemInfo* i, double x, double y);
double findDistance(LineItemInfo* i, double x, double y, long long& index);

class JustAplot: public QDialog {
	Q_OBJECT
public:
	std::string key;
	std::string name;
	JPType type;

	JustAplot(const std::string& key_, XQPlots* pf_, QWidget *parent, JPType type_);
	
	/**   it will 'delete' all the lines itself  
	*/
	virtual ~JustAplot();
	virtual void title(const std::string& s);
	virtual void footer(const std::string& s);
	virtual void xlabel(const std::string&  s) = 0;
	virtual void ylabel(const std::string&  s) = 0;
	virtual void zlabel(const std::string&  s) {}
	virtual void drawMarker(double X, double Y, int type = 1) = 0;
	virtual void drawMarker(double t);
	virtual void onClip(double t1, double t2);

	/** Just save this pointer.
	it will 'delete' all the pointers itself later

	*/
	virtual void addLine(LineItemInfo* line);
	virtual void addMesh(const MeshInfo& info) {}
	virtual void makeMarkersVisible(bool visible);
	virtual void replot() {}

signals:
	void exiting(const std::string&);
	void onSelection(const std::string& key_);
	void onPicker(const std::string& key_, double X, double Y);

protected:
	std::list<LineItemInfo*> linesInfo;
	XQPlots* pf;
	LineItemInfo* cInfo; ///< current line info
};


#endif

