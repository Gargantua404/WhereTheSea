#include "dataprocessor.h"

DataProcessor::DataProcessor(const QSettings & settings, QObject * parent):QObject(parent),logFile_(NULL),RadarProccessor_(),localTime_(QDateTime::currentDateTime()){
    imageDirStr_= settings.value("/Settings/ImageDir","").toString();
    logFileStr_=settings.value("/Settings/LogFile","").toString();
    outputFileStr_=settings.value("/Settings/OutputFile","").toString();
    imageDir_.setCurrent(imageDirStr_);
    frequency_=settings.value("/Settings/Frequency",15).toInt();
    logFile_=fopen(logFileStr_.toStdString().c_str(),"w");

    if(logFile_!=NULL){
        fprintf(logFile_,"Log file <%s> creation: %s\n"
                    "Directory with images: %s\n"
                    "Output file with objects' motion data: %s\n"
                    "Minimal anount of files to perform: %i\n\n",                logFileStr_.toStdString().c_str(),localTime_.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str(), imageDirStr_.toStdString().c_str(), outputFileStr_.toStdString().c_str(), frequency_);
    }

    dirModel_=new QFileSystemModel;
    dirModel_->setRootPath(imageDirStr_);
    dirModel_->setFilter(QDir::Files);
    dirModel_->setNameFilters(QStringList()<<"*.bmp");
    dirModel_->setNameFilterDisables(false);

    RadarProccessor_.setLogFile(logFile_);
    RadarProccessor_.setOutputFile(outputFileStr_.toStdString());
    RadarProccessor_.setFreq(frequency_);

    state_=0;
}

DataProcessor::~DataProcessor(){
    if(logFile_!=NULL){
        fclose(logFile_);
    }
}

void DataProcessor::changeButtonApply(int state){
    //input parameter state in external format
    if(!imageDir_.exists(imageDirStr_)){
        if(logFile_!=NULL){
            fprintf(logFile_,"Incorrect path to the directory with images\n");
        }
        throwError(0);
        return;
    }
    else if(!QFile(logFileStr_).exists(logFileStr_)){
        throwError(1);
        return;

    }
    else if(!QFile(outputFileStr_).exists(outputFileStr_)){
        if(logFile_!=NULL){
            fprintf(logFile_,"Incorrect path to the output file with objects' motion data with images\n");
        }
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
        if(logFile_!=NULL){
            fprintf(logFile_,"Path to the directory with images was changed to %s\n",imageDirStr_.toStdString().c_str());
        }
    }
    if (logFileStr_!=logFile){
        if(logFile_!=NULL){
             fprintf(logFile_,"Path to the log file was changed to <%s> \n",logFileStr_.toStdString().c_str());
            fclose(logFile_);
        }
        logFileStr_=logFile;
        logFile_=fopen(logFileStr_.toStdString().c_str(),"w");
        RadarProccessor_.setLogFile(logFile_);
        if(logFile_!=NULL){
            localTime_=QDateTime::currentDateTime();
            fprintf(logFile_,"Log file <%s> creation: %s\n"
                        "Directory with images: %s\n"
                        "Output file with objects' motion data: %s\n"
                        "Minimal anount of files to perform: %i\n\n",                logFileStr_.toStdString().c_str(),localTime_.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str(), imageDirStr_.toStdString().c_str(), outputFileStr_.toStdString().c_str(), frequency_);
        }
    }
    if (outputFileStr_!=outFile){
        outputFileStr_=outFile;
        RadarProccessor_.setOutputFile(outputFileStr_.toStdString());
        if(logFile_!=NULL){
             fprintf(logFile_,"Path to the output file was changed to <%s> \n",outputFileStr_.toStdString().c_str());
        }
    }
    if (frequency_!=freq){
        frequency_=freq;
        RadarProccessor_.setFreq(frequency_);
        if(logFile_!=NULL){
             fprintf(logFile_,"New minimal anount of files to perform: %i\n",frequency_);
        }
    };
}

void DataProcessor::readAllImagesOnce(){
    QStringList FirstImages=  imageDir_.entryList(QStringList(), QDir::Files);
    if(logFile_!=NULL){
        localTime_=QDateTime::currentDateTime();
        fprintf(logFile_,"  Read images: %s\n",localTime_.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
    }
    for (int i=0; i<FirstImages.size(); ++i){
        imagePathesQueue_.enqueue(FirstImages.at(i));
        if(logFile_!=NULL){
            fprintf(logFile_,"   %s\n",FirstImages.at(i).toStdString().c_str());
        }
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
    if(logFile_!=NULL){
        localTime_=QDateTime::currentDateTime();
        fprintf(logFile_,"  Read images: %s\n",localTime_.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
    }
    for(int i=start; i<=end;++i ){
        QModelIndex PathIndex=dirModel_->index(i,0,parent);
        QVariant PathName= PathIndex.data();
        imagePathesQueue_.enqueue(PathName.toString());
        if(logFile_!=NULL){
            fprintf(logFile_,"   %s\n",PathName.toString().toStdString().c_str());
        }
    }
    perform(state_);
}
