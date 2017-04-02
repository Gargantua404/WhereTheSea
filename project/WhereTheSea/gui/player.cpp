#include "player.h"

Player::Player (QWidget *parent):QMainWindow(parent){
    this->setWindowTitle(tr("Where the sea"));
    this->setFixedSize(240,170);

    //menu
    settingsMenu_ = new SettingsMenu(tr("&Settings"),this);
    helpMenu_ = new QMenu (tr("&Help"),this);

    // actions
    toSettings_ = new QAction(tr("Common settings"),settingsMenu_);
    settingsMenu_->addAction(toSettings_);
    //settings window
    setWin_ = new SettingsWindow("Aa","Bb","Cc",10,settingsMenu_);

    //buttons
    widgetForLayout_= new QWidget(this);
    pbxLayout_ =new QBoxLayout(QBoxLayout::LeftToRight,widgetForLayout_);

    buttonOnPause_ = new PausePlayButton(widgetForLayout_);
    buttonStop_ = new StopButton(widgetForLayout_);

    pbxLayout_->addWidget(buttonOnPause_);
    pbxLayout_->addWidget(buttonStop_);

    widgetForLayout_->setLayout(pbxLayout_);

    //statusBar
    stBar_ =new StatBar(this);

    //assembling
    menuBar()->addMenu(settingsMenu_);
    menuBar()->addMenu(helpMenu_);
    setCentralWidget(widgetForLayout_);
    statusBar()->addWidget(stBar_);

    processor_ = new DataProcessor(this);

    // changes processor state
    connect(buttonOnPause_,SIGNAL(clicked(bool)),buttonOnPause_,SLOT(informAll(bool)));
    connect(buttonOnPause_,SIGNAL(pausePlayButtonClicked(int)),processor_,SLOT(changeButtonApply(int)));

    connect(buttonStop_,SIGNAL(clicked(bool)),buttonStop_,SLOT(informAll(bool)));
    connect(buttonStop_,SIGNAL(stopButtonClicked(int)),processor_,SLOT(changeButtonApply(int)));

    //inner gui connections
    connect (toSettings_,SIGNAL(triggered()), setWin_, SLOT(openSettingsWindow()));
    //OPEN SETTINGS WINDOW
    //OPEN HELP WINDOW

    //changes gui view
    connect (processor_,SIGNAL(changeStateView(int)), stBar_,SLOT(changeState(int)));
    connect (processor_,SIGNAL(changeStateView(int)),settingsMenu_,SLOT(changeState(int)));
    //SEND SIGNAL TO SETTINS WINDOW TO HIDE SOME PARAMETERS
    connect (processor_,SIGNAL(changeStateView(int)),buttonOnPause_,SLOT(switchView(int)));

    //let frequency change during pause
}

SettingsWindow::SettingsWindow(QString imDir,QString logFile,QString outputFile, int frequency,QWidget * parent):imageDir_(imDir),logFile_(logFile),outputFile_(outputFile),frequency_(frequency),QDialog(parent){
    this->setWindowTitle(tr("Settings"));
    this->setModal(true);
    this->setWindowFlags(Qt::Window);
    this->setFixedSize(510,400);

    //geometry settings
    int marginvertical=1;
    int spacingvertical=5;
    int spacinghorizontal =3;

    // Pathes area
    QGroupBox * gPathes = new QGroupBox(tr("Pathes"),this);
    gPathes->setGeometry(5,10,500,100);

    QVBoxLayout * pathesLayout= new QVBoxLayout(this);
    pathesLayout->setMargin(marginvertical);
    pathesLayout->setSpacing(spacingvertical);

    QBoxLayout * ImageDirLayout = new QBoxLayout (QBoxLayout::LeftToRight);
    ImageDirLayout->setMargin(marginvertical);
    ImageDirLayout->setSpacing(spacinghorizontal);

    QLabel * label1= new QLabel(tr("Directory with images:"));
    imageDirLine_ = new QLineEdit;

    imageDirLine_->setText(imageDir_);
    QPushButton * but1= new QPushButton(tr("Browse"));

    ImageDirLayout->addWidget(label1);
    ImageDirLayout->addWidget(imageDirLine_);

    ImageDirLayout->addWidget(but1);

    QBoxLayout * LogFileLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    LogFileLayout->setMargin(marginvertical);
    LogFileLayout->setSpacing(spacinghorizontal);

    QLabel * label2= new QLabel(tr("Log file:"));
    logFileLine_ = new QLineEdit;
    logFileLine_->setText(logFile_);
    QPushButton * but2= new QPushButton(tr("Browse"));

    LogFileLayout->addSpacing(68);
    LogFileLayout->addWidget(label2);
    LogFileLayout->addWidget(logFileLine_);
    LogFileLayout->addWidget(but2);

    QBoxLayout * OutputFileLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    OutputFileLayout->setMargin(marginvertical);
    OutputFileLayout->setSpacing(spacinghorizontal);

    QLabel * label3= new QLabel(tr("Output file:"));
    outputFileLine_ = new QLineEdit;
    outputFileLine_->setText(outputFile_);
    QPushButton * but3= new QPushButton(tr("Browse"));

    OutputFileLayout->addSpacing(51);
    OutputFileLayout->addWidget(label3);
    OutputFileLayout->addWidget(outputFileLine_);
    OutputFileLayout->addWidget(but3);

    //assembling pathes area
    pathesLayout->addLayout(ImageDirLayout);
    pathesLayout->addLayout(LogFileLayout);
    pathesLayout->addLayout(OutputFileLayout);

    gPathes->setLayout(pathesLayout);

    //Common area
    QGroupBox * gCommon = new QGroupBox(tr("Common"),this);
    gCommon->setGeometry(5,120,500,50);

    QVBoxLayout * commonLayout= new QVBoxLayout(this);
    commonLayout->setMargin(marginvertical);
    commonLayout->setSpacing(spacingvertical);

    QBoxLayout * FrequencyLayout = new QBoxLayout (QBoxLayout::LeftToRight);
    FrequencyLayout->setMargin(marginvertical);
    FrequencyLayout->setSpacing(spacinghorizontal);

    QLabel * flabel= new QLabel(tr("Frequency (Hz):"));
    freqBox_ = new QSpinBox;

    freqBox_->setValue(frequency_);
    freqBox_->setFixedWidth(50);
    freqBox_->setRange(1,100);

    FrequencyLayout->addSpacing(30);
    FrequencyLayout->addWidget(flabel);
    FrequencyLayout->addWidget(freqBox_);
    FrequencyLayout->addStretch(1);

    commonLayout->addLayout(FrequencyLayout);

    gCommon->setLayout(commonLayout);

    //buttons
    QWidget * gButtons = new QWidget(this);
    gButtons->setGeometry(5,350,500,50);

    QBoxLayout * ButtonsLayout = new QBoxLayout (QBoxLayout::LeftToRight);
    ImageDirLayout->setMargin(marginvertical);
    ImageDirLayout->setSpacing(spacinghorizontal);

    QPushButton * bok = new QPushButton(tr("Ok"));
    QPushButton * bcancel = new QPushButton(tr("Cancel"));

    ButtonsLayout->addSpacing(350);
    ButtonsLayout->addWidget(bok);
    ButtonsLayout->addWidget(bcancel);
    ButtonsLayout->addStretch(1);

    gButtons->setLayout(ButtonsLayout);

    //connections
    connect(bok, SIGNAL(clicked(bool)),this,SLOT(applySetings()));
    connect(bcancel, SIGNAL(clicked(bool)),this,SLOT(cancelSettings()));

    //send initial parameters value to processor
    emit sendSettingsToProcessor(imageDir_,logFile_,outputFile_,frequency_);
}

void SettingsWindow::openSettingsWindow(){
    //block parent

    this->show();
}
int SettingsWindow::isFileSpecification(const QString &str){
    //returns 0 if the input string may be the path to a file (new or existed) and -1 if it's not.
    //algorithm: devide the input string into 2 parts: dir and file name. Check both to be valid.


    return 0;
}

void SettingsWindow::applySetings(){
    //constraints checking
    bool allisok=true;

    QString imageDirTemp= imageDirLine_->text();
    if(!QDir(imageDirTemp).exists()){
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the directory with images"),QMessageBox::Ok);
        allisok=false;
    }
    else{
        imageDir_= imageDirTemp;
    }

    QString logFileTemp= logFileLine_->text();

    //check file specification

    if(!isFileSpecification(logFileTemp)){
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the log-file"),QMessageBox::Ok);
        allisok=false;
    }
    else{
        logFile_=logFileTemp;
    }

    QString  outputFileTemp = outputFileLine_->text();
    if(!isFileSpecification(outputFileTemp)){
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the output file"),QMessageBox::Ok);
        allisok=false;
    }
    else{
        outputFile_=outputFileLine_->text();
    }

    frequency_=freqBox_->value();
    if(allisok){
        emit sendSettingsToProcessor(imageDir_,logFile_,outputFile_,frequency_);
        this->hide();
    }
}

void SettingsWindow::cancelSettings(){
    //Are QLineEdit being deleted themself?
    this->hide();
}

StatBar::StatBar(QWidget *parent):QLabel(parent),stateArr_({tr("Stopped") , tr("Paused"), tr("Working")}){
    this->setText(stateArr_.at(0)); //initialize by "stopped" value
}

void StatBar::changeState(int s){
    this->setText(stateArr_.at(s));
    this->repaint();
}


PausePlayButton::PausePlayButton(QWidget *parent):QPushButton(parent),state(0),IconsPath_({":/img/on.png",":/img/pause.png"}){
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


StopButton::StopButton(QWidget *parent):QPushButton(parent),IconPath_({":/img/off.png"}){
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
