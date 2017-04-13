#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QDir>
#include <QFileSystemWatcher>

#include "player.h"

class ActiveQueueString: public QObject,QQueue<QString>{
    Q_OBJECT
public:
    ActiveQueueString(QObject * parent);
signals:
    void notEmpty();
public slots:
    void ActivePush(const QString &); //send signal if size is increasing

};

class DataProcessor : public QObject
{
    Q_OBJECT
private:
    QString imageDirStr_;
    QString logFileStr_;
    QString outputFileStr_;

    QDir imageDir_;
    QFile logFile_;
    QFile outputFile_;

    int state_; //0-stopped, 1-paused, 2-working
    int frequency_; //frequency the dir are being looked over

    ActiveQueueString * imagePathesQueue_;
    QFileSystemWatcher * dirWatcher_;

    QDateTime lastFileDateMod; //the date of the modification of the last file in a queue
    bool isPerforming;
public:
    explicit DataProcessor(const QSettings &, QObject * parent=nullptr);
    //DataProcessor(QString imDir ,QString logFile,QString outFile,int frequency, QObject *parent =nullptr);
    ~DataProcessor();

/*    int getState(){
        return state_;
    }*/

    //algorithms slots
    /*void start();
    void pause();
    void stop();*/
    void perform();

signals:
    void changeStateView(int); //cause changes in gui view
public slots:
    //GUI slots
    void changeButtonApply(int); //apply signals from buttons
    void changedParametersApply(QString,QString,QString,int);

    void readImagePathes(); //watch dir and populate the queue

};

#endif // DATAPROCESSOR_H
