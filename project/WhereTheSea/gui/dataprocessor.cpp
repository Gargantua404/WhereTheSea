#include "dataprocessor.h"


ActiveQueueString::ActiveQueueString(QObject *parent):QObject(parent),QQueue<QString>(){

}

void ActiveQueueString::ActivePush(const QString &){

}

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
    dirWatcher_->addPath(imageDirStr);

    lastFileDateMod =  QDateTime::fromString("M1d1y0000:00:00","'M'M'd'd'y'yyhh:mm:ss");
    isPerforming=false;

    connect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
}

/*DataProcessor::DataProcessor(QString imDir ,QString logFile,QString outFile,int frequency, QObject *parent): DataProcessor(parent){
    imageDirStr_=imDir;
    logFileStr_=logFile;
    outputFileStr_=outFile;
    frequency_=frequency;
    imageDir_.setCurrent(imageDirStr_);
    logFile_.setFileName(logFileStr_);
    outputFile_.setFileName(outputFileStr_);
}
*/

DataProcessor::~DataProcessor(){
}

void DataProcessor::changeButtonApply(int state){
    //input parameter state in external format
    //DO ALL CHECK TO CHANGE THE STATE HERE(or in appropriate slots)
    //and here hust send signals


    /*if(!isPerforming){
        emit changeStateView(state_);
    }
    else{
        isPerforming=false;
    }*/
    int state_prev=state_;
    state_=state;

    this->blockSignals(true);
    if(state==0){
        if(state_prev==1){
            //last state was paused
            //state_=state; //temporary assignment here
            perform(state);
        }
        //state_=state; //temporary assignment here
        disconnect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
        this->blockSignals(false);
        emit changeStateView(state_);
    }
    else if(state==1){
        state_=state; //temporary assignment here
        disconnect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
        this->blockSignals(false);
        emit changeStateView(state_);
    }
    else{
        readImagePathes();
        connect(dirWatcher_,SIGNAL(directoryChanged(QString)),this,SLOT(readImagePathes()));
        state_=state; //temporary assignment here
        this->blockSignals(false);
        emit changeStateView(state_);
        perform(state);
    }

    //state_=state; //temporary assignment here
    //this->blockSignals(false);
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
    //isPerforming =true;
    while(state!=1 && !imagePathesQueue_->isEmpty()){
        QDebug << QTime::currentTime();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);//MUFFLE
      //  if(isPerforming)
        {
            //condition is for not to check twice
            //CHECK ALL SIGNALS
        }
    }
    /*if(!isPerforming){
        emit changeStateView(state_);
    }*/
    //isPerforming=false;
}

void DataProcessor::readImagePathes(){
    /*imageDir_.setFilter();
    QList<QString> str = imageDir_.entryList(".png",QDir::Files,QDir::Time);*/
    imagePathesQueue_->push_back("C:/test.txt"); // MUFFLE
}
