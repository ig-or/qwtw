
#include <QDialog>
//#include <QMainWindow>

class QMGL1;
class QMGL2;

class MGLTest: public QDialog {
	Q_OBJECT
public:
	MGLTest(QWidget * parent1 = 0);
private:
	QMGL1* test1;
	QMGL1* test12;
	QMGL2* test2;

private slots:
	void test();	
};
