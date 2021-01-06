
#include <QDialog>
//#include <QMainWindow>

class QMGL;

class MGLTest: public QDialog {
	Q_OBJECT
public:
	MGLTest(QWidget * parent1 = 0);
private:
	QMGL* test;
};
