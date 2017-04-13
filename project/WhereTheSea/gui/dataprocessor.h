#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QDir>
#include <QFileSystemWatcher>

#include "player.h"

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

    QQueue<QString> imagePathesQueue_;
    QFileSystemWatcher * dirWatcher_;

    //COULD BE USELESS
    QDateTime lastFileDateMod; //the date of the modification of the last file in a queue
public:
    explicit DataProcessor(const QSettings &, QObject * parent=nullptr);
    ~DataProcessor();

    //algorithms slots
    void perform(int state);

signals:
    void changeStateView(int); //cause changes in gui view
    void throwError(int); //0 - incorrect pathes to files or dirs
public slots:
    //GUI slots
    void changeButtonApply(int); //apply signals from buttons
    void changedParametersApply(QString,QString,QString,int);

    void readImagePathes(); //watch dir and populate the queue

};

#endif // DATAPROCESSOR_H
