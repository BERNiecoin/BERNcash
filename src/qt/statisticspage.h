#ifndef STATISTICSPAGE_H
#define STATISTICSPAGE_H

#include "clientmodel.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"
#include "blockbrowser.h" // some shared functions
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QSettings>
#include <QSlider>

namespace Ui {
class StatisticsPage;
}
class ClientModel;

class StatisticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPage(QWidget *parent = 0);
    ~StatisticsPage();
    
    void setModel(ClientModel *model);
    
    int heightPrevious;
    int connectionPrevious;
    int volumePrevious;
    int stakeminPrevious;
    int stakemaxPrevious;
    QString stakecPrevious;
    double rewardPrevious;
    double netPawratePrevious;
    QString pawratePrevious;
    double hardnessPrevious;
    double hardnessPrevious2;
    
public slots:

    void updateStatistics();
    void updatePrevious(int, int, int, double, double, double, double, QString, int, int);
    void timerCountDown();

private slots:

    void on_startButton_released();

private:
    Ui::StatisticsPage *ui;
    ClientModel *model;
    
};

#endif // STATISTICSPAGE_H
