

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
	int iKey;		///< plot ID ?
	std::string name;
	int clipGroup;  ///< defaults to 0
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
	virtual void draw3DMarker(const CBPickerInfo& cpi) {};
	virtual void drawMarker(double t);
	virtual void onClip(double t1, double t2);
	virtual void removeLine(LineItemInfo* line);
	void remove_lines();
	virtual void changeLine(LineItemInfo* line, double* x, double* y, double* z, double* time, int size) {}

	/** Just save this pointer.
	it will 'delete' all the pointers itself later

	*/
	virtual void addLine(LineItemInfo* line);
	virtual void addMesh(const MeshInfo& info) {}
	virtual void makeMarkersVisible(bool visible);
	virtual void replot() {}
	virtual void addVMarker(double t, const char* label = 0, int id_ = 0) {}
	virtual void removeVMarker(int id_) {}
	bool looksLikeTopView() {
		return maybeTopView; 
	}

signals:
	void exiting(const std::string&);
	void onSelection(const std::string& key_);
	void onPicker(const std::string& key_, double X, double Y);

protected:
	bool maybeTopView = false;
	std::list<LineItemInfo*> linesInfo;
	XQPlots* pf;
	LineItemInfo* cInfo; ///< current line info
};


#endif

