#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QDir>
#include <QFileSystemWatcher>

#include "player.h"
#include "../core/Radar.h"
class DataProcessor : public QObject
{
    Q_OBJECT
private:
    QString imageDirStr_;
    QString logFileStr_;
    QString outputFileStr_;

    QDir imageDir_;
    FILE * logFile_;

    int state_; //0-stopped, 1-paused, 2-working
    int logFileBox_;
    int minFile_; //minimal number of files to process
    int scale_;
    double identThreshold_;

    QQueue<QString> imagePathesQueue_;
    QFileSystemModel * dirModel_;

    Radar RadarProccessor_;

    QDateTime localTime_;

    void writeToLogBegin(FILE *,QString logFileStr,QDateTime localTime,QString imageDirStr, QString outputFileStr, int minFile, int scale, double identThreshold);
public:
    explicit DataProcessor(const QSettings &, QObject * parent=nullptr);
    ~DataProcessor();

    //algorithms methods
    void readAllImagesOnce();
    void perform();

signals:
    void changeStateView(int); //cause changes in gui view
    void changeCounterView(int);
    void throwError(int); //0,1,2 - incorrect pathes to files or dirs

public slots:
    //GUI slots
    void changeButtonApply(int); //apply signals from buttons
    void changedParametersApply(QString,int,QString,int,int,double); //apply changes from setting window

    //algorithm slots
    void readImagesAndRun(const QModelIndex &, int, int);
};

#endif // DATAPROCESSOR_H
