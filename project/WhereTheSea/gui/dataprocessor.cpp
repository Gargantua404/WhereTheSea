#include "dataprocessor.h"

DataProcessor::DataProcessor(const QSettings & settings, QObject * parent):QObject(parent){
    imageDirStr_= settings.value("/Settings/ImageDir","").toString();
    logFileStr_=settings.value("/Settings/LogFile","").toString();
    outputFileStr_=settings.value("/Settings/OutputFile","").toString();
    imageDir_.setCurrent(imageDirStr_);
    logFile_.setFileName(logFileStr_);
    outputFile_.setFileName(outputFileStr_);
    state_=0;
    frequency_=settings.value("/Settings/Frequency",15).toInt();

    dirWatcher_= new QFileSystemWatcher(this);
    dirWatcher_->addPath(imageDirStr_);

    lastFileDateMod =  QDateTime::fromString("M1d1y0000:00:00","'M'M'd'd'y'yyhh:mm:ss");

    connect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
}

DataProcessor::~DataProcessor(){
}

void DataProcessor::changeButtonApply(int state){
    //input parameter state in external format
    if(!imageDir_.exists(imageDirStr_)){
        throwError(0);
        return;
    }
    else if(!logFile_.exists(logFileStr_)){
        throwError(1);
        return;

    }
    else if(!outputFile_.exists(outputFileStr_)){
        throwError(2);
        return;
    }

    //and here hust send signals

    int state_prev=state_;
    state_=state;

    this->blockSignals(true);
    if(state_==0 || state_==1){
        if(state_==0){
            if(state_prev==1 || state_prev==2){
                perform(state_);
            }
        }
        disconnect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
        this->blockSignals(false);
        emit changeStateView(state_);
    }
    else{
        readImagePathes();
        connect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
        this->blockSignals(false);
        emit changeStateView(state_);
        perform(state_);
    }
}

void DataProcessor::changedParametersApply(QString imDir, QString logFile, QString outFile, int freq) {
    dirWatcher_->removePath(imageDirStr_);

    imageDirStr_=imDir;
    logFileStr_=logFile;
    outputFileStr_=outFile;
    frequency_=freq;

    imageDir_.setCurrent(imageDirStr_);
    logFile_.setFileName(logFileStr_);
    outputFile_.setFileName(outputFileStr_);

    dirWatcher_->addPath(imageDirStr_);
}

void DataProcessor::perform(int state){
    while(state!=1 && !imagePathesQueue_.isEmpty()){
        qDebug() << QTime::currentTime();
        imagePathesQueue_.dequeue();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);//MUFFLE
            //condition is for not to check twice
            //CHECK ALL SIGNALS
    }
}

void DataProcessor::readImagePathes(){
    /*imageDir_.setFilter();
    QList<QString> str = imageDir_.entryList(".png",QDir::Files,QDir::Time);*/
    imagePathesQueue_.enqueue("C:/test.txt"); // MUFFLE
    imagePathesQueue_.enqueue("C:/test2.txt"); // MUFFLE
    imagePathesQueue_.enqueue("C:/test3.txt"); // MUFFLE
}
