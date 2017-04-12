#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
    state_=0;
}

DataProcessor::DataProcessor(QString imDir ,QString logFile,QString outFile,int frequency, QObject *parent):QObject(parent),imageDir_(imDir),logFile_(logFile),outputFile_(outFile),frequency_(frequency){
    state_=0;
}

void DataProcessor::changeButtonApply(int state){
    //input parameter state in external format
    //DO ALL CHECK TO CHANGE THE STATE HERE(or in appropriate slots)
    //and here hust send signals
    if(state==0){
        //SEND SIGNAL TO STOP DATA PERFORM
    }
    else if(state==1){
        //send signal pause
    }
    else{
        //send signal to begin data performing
    }

    state_=state; //temporary assignment here
    emit changeStateView(state_);
}

void DataProcessor::changedParametersApply(QString imDir, QString logFile, QString outFile, int freq) {
    imageDir_=imDir;
    logFile_=logFile;
    outputFile_=outFile;
    frequency_=freq;
}
