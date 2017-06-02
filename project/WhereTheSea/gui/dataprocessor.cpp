#include "dataprocessor.h"

DataProcessor::DataProcessor(const QSettings & settings, QObject * parent):QObject(parent),logFile_(NULL),
    scale_(settings.value("/Settings/Scale","").toInt()),
    RadarProccessor_(scale_), localTime_(QDateTime::currentDateTime()){

    imageDirStr_= settings.value("/Settings/ImageDir","").toString();
    outputFileStr_=settings.value("/Settings/OutputFile","").toString();
    imageDir_.setCurrent(imageDirStr_);
    minFile_=settings.value("/Settings/MinFile",15).toInt();
    identThreshold_=settings.value("/Settings/IdentThreshold",0.05).toDouble();

    logFileBox_=settings.value("/Settings/LogFile",0).toInt();
    if(logFileBox_==2){
        logFileStr_=QCoreApplication::applicationDirPath()+"/WhereTheSea_"+ localTime_.toString("yyyyMMdd_hhmmss")+ ".log";
        logFile_=fopen(logFileStr_.toStdString().c_str(),"w");
        RadarProccessor_.setLogFile(logFile_);
     }
    writeToLogBegin(logFile_, logFileStr_,localTime_,imageDirStr_,outputFileStr_,minFile_,scale_,identThreshold_);

    dirModel_=new QFileSystemModel;
    dirModel_->setRootPath(imageDirStr_);
    dirModel_->setFilter(QDir::Files);
    dirModel_->setNameFilters(QStringList()<<"*.bmp");
    dirModel_->setNameFilterDisables(false);

    RadarProccessor_.setOutputFile(outputFileStr_.toStdString());
    RadarProccessor_.setFreq(minFile_);
    RadarProccessor_.setScale(scale_);
    RadarProccessor_.setIdentThreshold(identThreshold_);

    state_=0;
}

DataProcessor::~DataProcessor(){
    if(logFile_!=NULL){
        fclose(logFile_);
    }
}

void DataProcessor::writeToLogBegin(FILE * logFile, QString logFileStr, QDateTime localTime, QString imageDirStr, QString outputFileStr, int minFile, int scale , double identThreshold){
    if(logFile!=NULL){
        fprintf(logFile,"Log file <%s> creation: %s\n"
                    "Directory with images: %s\n"
                    "Output file with objects' motion data: %s\n"
                    "Minimal amount of files to perform: %i\n"
                    "Scale factor: %i\n"
                    "Identification threshold : %f\n\n",
logFileStr.toStdString().c_str(),localTime.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str(), imageDirStr.toStdString().c_str(), outputFileStr.toStdString().c_str(), minFile,scale,identThreshold);
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
    else if(!QFile(outputFileStr_).exists(outputFileStr_)){
        if(logFile_!=NULL){
            fprintf(logFile_,"Incorrect path to the output file with objects' motion data with images\n");
        }
        throwError(1);
        return;
    }

    int state_prev=state_;
    state_=state;

    this->blockSignals(true);
    if(state_==0 || state_==1){
        if(state_==0){
            if(state_prev==1 || state_prev==2){
                perform();
                //can be less then fequency images in queue
                imagePathesQueue_.clear(); //clear after stopping and performing residual images                
            }
        disconnect(dirModel_,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(readImagesAndRun(QModelIndex,int,int)));
        }
        this->blockSignals(false);
        emit changeCounterView(imagePathesQueue_.size());
        emit changeStateView(state_);
    }
    else{
        connect(dirModel_,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(readImagesAndRun(QModelIndex,int,int)));
        this->blockSignals(false);
        emit changeStateView(state_);
        if(state_prev==0){
            readAllImagesOnce();
        }
        perform();
    }
}

void DataProcessor::changedParametersApply(QString imDir,int logFileBox, QString outFile, int freq, int scale,double identThreshold) {
    if(imageDirStr_!=imDir){
        imageDirStr_=imDir;
        imageDir_.setCurrent(imageDirStr_);
        dirModel_->setRootPath(imageDirStr_);
        if(logFile_!=NULL){
            fprintf(logFile_,"Path to the directory with images was changed to %s\n",imageDirStr_.toStdString().c_str());
        }
    }

    if (outputFileStr_!=outFile){
        outputFileStr_=outFile;
        RadarProccessor_.setOutputFile(outputFileStr_.toStdString());
        if(logFile_!=NULL){
             fprintf(logFile_,"Path to the output file was changed to <%s> \n",outputFileStr_.toStdString().c_str());
        }
    }

    if (logFileBox_!=logFileBox){
        if(logFileBox==2){
            //box is checked
            localTime_=QDateTime::currentDateTime();
            logFileStr_=QCoreApplication::applicationDirPath()+"/WhereTheSea_"+ localTime_.toString("yyyyMMdd_hhmmss")+ ".log";
            logFile_=fopen(logFileStr_.toStdString().c_str(),"w");
            RadarProccessor_.setLogFile(logFile_);
            writeToLogBegin(logFile_, logFileStr_,localTime_,imageDirStr_,outputFileStr_,minFile_,scale_,identThreshold_);
        }
        else{
             if(logFile_!=NULL){
                 localTime_=QDateTime::currentDateTime();
                 fprintf(logFile_,"Log file keeping was disabled: %s\n",  localTime_.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
                 fclose(logFile_);
                 logFile_=NULL;
             }
        }
        logFileBox_=logFileBox;
    }

    if (minFile_!=freq){
        minFile_=freq;
        RadarProccessor_.setFreq(minFile_);
        if(logFile_!=NULL){
             fprintf(logFile_,"New minimal anount of files to perform: %i\n",minFile_);
        }
    }

    if (scale_!=scale){
        scale_=scale;
        RadarProccessor_.setScale(scale_);
        if(logFile_!=NULL){
             fprintf(logFile_,"New scale : %i\n",scale_);
        }
    }

    if (identThreshold_!=identThreshold){
        identThreshold_=identThreshold;
        RadarProccessor_.setIdentThreshold(identThreshold_);
        if(logFile_!=NULL){
             fprintf(logFile_,"New identification threshold: %f\n",identThreshold_);
        }
    }

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
    emit changeCounterView(FirstImages.size());
}

void DataProcessor::perform(){
    while(state_!=1 && imagePathesQueue_.size()>=minFile_){
        list<string> img_files_to_pass;
        for(int j=0;j<minFile_;++j){
            img_files_to_pass.push_back(imagePathesQueue_.first().toStdString());
            //qDebug() << imagePathesQueue_.first();
            imagePathesQueue_.dequeue();
            emit changeCounterView(imagePathesQueue_.size());
        }
        RadarProccessor_.run(img_files_to_pass);

        QCoreApplication::processEvents(QEventLoop::AllEvents);
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
        emit changeCounterView(imagePathesQueue_.size());
        if(logFile_!=NULL){
            fprintf(logFile_,"   %s\n",PathName.toString().toStdString().c_str());
        }
    }
    perform();
}
