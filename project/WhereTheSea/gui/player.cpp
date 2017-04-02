#include "player.h"

Player::Player (){
    this->setWindowTitle("Where the sea");
    this->setFixedSize(240,170);

    //menu
    settingsMenu_ = new SettingsMenu("&Settings");
    helpMenu_ = new QMenu ("&Help");

    // actions
    toSettings_ = new QAction("Common settings");
    QObject::connect (toSettings_,SIGNAL(triggered()), this, SLOT(OpenSettingsWindow()));
    settingsMenu_->addAction(toSettings_);

    //buttons
    buttonOnPause_ = new PausePlayButton;
    buttonStop_ = new StopButton;

    pbxLayout_ =new QBoxLayout(QBoxLayout::LeftToRight);
    pbxLayout_->addWidget(buttonOnPause_);
    pbxLayout_->addWidget(buttonStop_);

    widgetForLayout_= new QWidget();
    widgetForLayout_->setLayout(pbxLayout_);

    //statusBar
    stBar_ =new StatBar;

    //assembling
    menuBar()->addMenu(settingsMenu_);
    menuBar()->addMenu(helpMenu_);
    setCentralWidget(widgetForLayout_);
    statusBar()->addWidget(stBar_);

    processor_ = new DataProcessor;

    // changes processor state
    connect(buttonOnPause_,SIGNAL(clicked(bool)),buttonOnPause_,SLOT(informAll(bool)));
    connect(buttonOnPause_,SIGNAL(pausePlayButtonClicked(int)),processor_,SLOT(changeButtonApply(int)));

    connect(buttonStop_,SIGNAL(clicked(bool)),buttonStop_,SLOT(informAll(bool)));
    connect(buttonStop_,SIGNAL(stopButtonClicked(int)),processor_,SLOT(changeButtonApply(int)));

    //inner gui connections
    //OPEN SETTINGS WINDOW
    //OPEN HELP WINDOW

    //changes gui view
    connect (processor_,SIGNAL(changeStateView(int)), stBar_,SLOT(changeState(int)));
    connect (processor_,SIGNAL(changeStateView(int)),settingsMenu_,SLOT(changeState(int)));
    //SEND SIGNAL TO SETTINS WINDOW TO HIDE SOME PARAMETERS
    connect (processor_,SIGNAL(changeStateView(int)),buttonOnPause_,SLOT(switchView(int)));

    //let frequency change during pause
}

void Player::OpenSettingsWindow(){

}

StatBar::StatBar():stateArr_({"Stopped" , "Paused", "Working"}){
    this->setText(stateArr_.at(0)); //initialize by "stopped" value
}

void StatBar::changeState(int s){
    this->setText(stateArr_.at(s));
    this->repaint();
}


PausePlayButton::PausePlayButton():state(0),IconsPath_({"../../WhereTheSea/img/on.png","../../WhereTheSea/img/pause.png"}){
    ButtonPicture_.reserve(2);
    ButtonIcon_.reserve(2);

    for(int i=0; i<2;++i){
        if(QFileInfo(IconsPath_[i]).exists()){
            ButtonPicture_.push_back(new QPixmap(IconsPath_[i]));
            ButtonIcon_.push_back(new QIcon(*ButtonPicture_[i]));
        }
        else{
            qDebug()<<"no file";
            //return error
        }
    }
    this->setIcon(*ButtonIcon_[0]);
    this->setIconSize(ButtonPicture_[0]->rect().size());
}

void PausePlayButton::informAll(bool){
    state=!state;
    emit pausePlayButtonClicked(state+1);
    //inner state =0 (paused) -> external state = 1
    //inner state =1 (working) -> external state = 2
}

void PausePlayButton::switchView(int state){
    //input parameter state in external format
    int inner_state=0;
    if (state ==0 || state ==1){
        inner_state=0;
    }
    else {
        inner_state=1;
    }
    this->setIcon(*ButtonIcon_[inner_state]);
    this->setIconSize(ButtonPicture_[inner_state]->rect().size());
}


StopButton::StopButton():IconPath_({"../../WhereTheSea/img/off.png"}){
    ButtonPicture_ = new QPixmap(IconPath_);
    ButtonIcon_ = new QIcon(*ButtonPicture_);
    this->setIcon(*ButtonIcon_);
    this->setIconSize(ButtonPicture_->rect().size());

}

void StopButton::informAll(bool){
    emit stopButtonClicked(0);
    //state 0 corresponds to "stopped" in processor class
}


void SettingsMenu::changeState(int state){
    //s in ext
    if (state==0 || state==1){
        this->setDisabled(false);
    }
    else{
        this->setDisabled(true);
    }
}
