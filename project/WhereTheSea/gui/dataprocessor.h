#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QDir>

#include "player.h"

class DataProcessor : public QObject
{
    Q_OBJECT
private:
    QDir imageDir_;
    QString logFile_;
    QString outputFile_;

    int state_; //0-stopped, 1-paused, 2-working
    int frequency_; //frequency the dir are being looked over
public:
    explicit DataProcessor(QObject *parent = nullptr);
    DataProcessor(QString imDir ,QString logFile,QString outFile,int frequency, QObject *parent =nullptr);

    int getState(){
        return state_;
    }
signals:
    void changeStateView(int); //cause changes in gui view
public slots:
    void changeButtonApply(int); //apply signals from buttons
    void changedParametersApply(QString,QString,QString,int);
};

#endif // DATAPROCESSOR_H
