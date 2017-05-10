#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QDir>
#include <QFileSystemWatcher>

#include "player.h"
#include "../core/Radar2.h"
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
    int frequency_; //minimal number of files to process

    QQueue<QString> imagePathesQueue_;
    QFileSystemModel * dirModel_;

    Radar RadarProccessor_;

    QDateTime localTime_;
public:
    explicit DataProcessor(const QSettings &, QObject * parent=nullptr);
    ~DataProcessor();

    //algorithms methods
    void readAllImagesOnce();
    void perform(int state);

signals:
    void changeStateView(int); //cause changes in gui view
    void throwError(int); //0,1,2 - incorrect pathes to files or dirs

public slots:
    //GUI slots
    void changeButtonApply(int); //apply signals from buttons
    void changedParametersApply(QString,QString,QString,int); //apply changes from setting window

    //algorithm slots
    void readImagesAndRun(const QModelIndex &, int, int);
};

#endif // DATAPROCESSOR_H
