#include "dataprocessor.h"

DataProcessor::DataProcessor(QObject *parent) : QObject(parent){
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

