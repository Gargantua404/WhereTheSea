#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QDir>

class DataProcessor : public QObject
{
    Q_OBJECT
private:
    int state_; //0-stopped, 1-paused, 2-working
    QDir dirProcess_; //dir with files of images
    int frequency_; //frequency the dir are being looked over
public:
    explicit DataProcessor(QObject *parent = 0);

signals:
    void changeStateView(int); //cause changes in gui view
public slots:
    void changeButtonApply(int); //apply signals from buttons
};

#endif // DATAPROCESSOR_H
