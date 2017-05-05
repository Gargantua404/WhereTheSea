#include "dataprocessor.h"

DataProcessor::DataProcessor(const QSettings & settings, QObject * parent):QObject(parent),RadarProccessor_(){
    imageDirStr_= settings.value("/Settings/ImageDir","").toString();
    logFileStr_=settings.value("/Settings/LogFile","").toString();
    outputFileStr_=settings.value("/Settings/OutputFile","").toString();
    imageDir_.setCurrent(imageDirStr_);
    logFile_=fopen(logFileStr_.toStdString().c_str(),"w");

    state_=0;
    frequency_=settings.value("/Settings/Frequency",15).toInt();

    dirModel_=new QFileSystemModel;
    dirModel_->setRootPath(imageDirStr_);
    dirModel_->setFilter(QDir::Files);

    RadarProccessor_.setPath(logFile_);
    RadarProccessor_.setPath(outputFileStr_.toStdString());
    RadarProccessor_.setFreq(frequency_);
}

DataProcessor::~DataProcessor(){
    if(logFile_!=NULL){
        fclose(logFile_);
    }
}

void DataProcessor::changeButtonApply(int state){
    //input parameter state in external format
    if(!imageDir_.exists(imageDirStr_)){
        throwError(0);
        return;
    }
    else if(!QFile(logFileStr_).exists(logFileStr_)){
        throwError(1);
        return;

    }
    else if(!QFile(outputFileStr_).exists(outputFileStr_)){
        throwError(2);
        return;
    }

    int state_prev=state_;
    state_=state;

    this->blockSignals(true);
    if(state_==0 || state_==1){
        if(state_==0){
            if(state_prev==1 || state_prev==2){
                perform(state_);
                //can be less then fequency images in queue
                imagePathesQueue_.clear(); //clear after stopping and performing residual images
            }
        disconnect(dirModel_,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(readImagesAndRun(QModelIndex,int,int)));
        }
        this->blockSignals(false);
        emit changeStateView(state_);
    }
    else{
        connect(dirModel_,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(readImagesAndRun(QModelIndex,int,int)));
        this->blockSignals(false);
        emit changeStateView(state_);
        if(state_prev==0){
            readAllImagesOnce();
        }
        perform(state_);
    }
}

void DataProcessor::changedParametersApply(QString imDir, QString logFile, QString outFile, int freq) {
    if(imageDirStr_!=imDir){
        imageDirStr_=imDir;
        imageDir_.setCurrent(imageDirStr_);
        dirModel_->setRootPath(imageDirStr_);
    }
    if (logFileStr_!=logFile){
        if(logFile_!=NULL){
            fclose(logFile_);
        }
        logFileStr_=logFile;
        logFile_=fopen(logFileStr_.toStdString().c_str(),"w");
        RadarProccessor_.setPath(logFile_);
    }
    if (outputFileStr_!=outFile){
        outputFileStr_=outFile;
        RadarProccessor_.setPath(outputFileStr_.toStdString());
    }
    if (frequency_!=freq){
        frequency_=freq;
        RadarProccessor_.setFreq(frequency_);
    };
}

void DataProcessor::readAllImagesOnce(){
    QStringList FirstImages=  imageDir_.entryList(QStringList(), QDir::Files);
    for (int i=0; i<FirstImages.size(); ++i){
           imagePathesQueue_.enqueue(FirstImages.at(i));
    }
}

void DataProcessor::perform(int state){
    while(state!=1 && imagePathesQueue_.size()>=frequency_){
        //MUFFLE
        list<string> img_files_to_pass;
        for(int j=0;j<frequency_;++j){
            img_files_to_pass.push_back(imagePathesQueue_.first().toStdString());
            qDebug() << imagePathesQueue_.first();
            imagePathesQueue_.dequeue();
        }
        RadarProccessor_.run(img_files_to_pass);

        QCoreApplication::processEvents(QEventLoop::AllEvents); //NOT TESTED
    }
}

void DataProcessor::readImagesAndRun(const QModelIndex& parent, int start, int end){
   for(int i=start; i<=end;++i ){
        QModelIndex PathIndex=dirModel_->index(i,0,parent);
        QVariant PathName= PathIndex.data();
        imagePathesQueue_.enqueue(PathName.toString());
    }
    perform(state_);
}
