#include "player.h"

Player::Player (QWidget *parent):QMainWindow(parent){
    //initialise stored  parameters
    storedSet_ = new QSettings("SPbSTU","WhereTheSea",this);

    this->setWindowTitle(tr("Where the sea"));
    this->setFixedSize(240,170);

    //create Thread
    ImThread_ = new ThreadImagePerform;
    ImThread_->start();

    //menu
    settingsMenu_ = new SettingsMenu(tr("&Settings"),this);
    helpMenu_ = new QMenu (tr("&Help"),this);

    // actions
    toSettings_ = new QAction(tr("Common settings"),settingsMenu_);
    settingsMenu_->addAction(toSettings_);
    //settings window
    setWin_ = new SettingsWindow(storedSet_,settingsMenu_);

    processor_ = new DataProcessor(*storedSet_); //don't pass the parent reference cause the "processor_" object is in another thread
    processor_->moveToThread(ImThread_);

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

    // changes processor state
    connect(buttonOnPause_,SIGNAL(clicked(bool)),buttonOnPause_,SLOT(informAll(bool)));
    connect(buttonOnPause_,SIGNAL(pausePlayButtonClicked(int)),processor_,SLOT(changeButtonApply(int)));

    connect(buttonStop_,SIGNAL(clicked(bool)),buttonStop_,SLOT(informAll(bool)));
    connect(buttonStop_,SIGNAL(stopButtonClicked(int)),processor_,SLOT(changeButtonApply(int)));

    //inner gui connections
    connect (toSettings_,SIGNAL(triggered()), setWin_, SLOT(openSettingsWindow()));
    //OPEN HELP WINDOW

    //changes gui view
    connect (processor_,SIGNAL(changeStateView(int)), stBar_,SLOT(changeState(int)));
    connect (processor_,SIGNAL(changeStateView(int)),settingsMenu_,SLOT(changeState(int)));
    connect (processor_,SIGNAL(changeStateView(int)),setWin_,SLOT(changeState(int)));
    connect (processor_,SIGNAL(changeStateView(int)),buttonOnPause_,SLOT(switchView(int)));

    //send new changed parameters to processor
    connect(setWin_,SIGNAL(sendSettingsToProcessor(QString,QString,QString,int)),processor_,SLOT(changedParametersApply(QString,QString,QString,int)));

    connect(processor_,SIGNAL(throwError(int)),this,SLOT(showMessage(int)));

}

void Player::showMessage(int type){
    switch(type){
    case 0:
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the directory with images"), QMessageBox::Ok);
        break;
    case 1:
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the log-file"),QMessageBox::Ok);
        break;
    case 2:
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the output file"),QMessageBox::Ok);
        break;
    }
}

Player::~Player(){
    //destruct thread
    ImThread_->quit();
    ImThread_->wait();
}

SettingsWindow::SettingsWindow(QSettings * storedSet, QWidget * parent):storedSet_(storedSet),QDialog(parent){
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
    imageDir_ = storedSet_->value("/Settings/ImageDir","").toString();

    imageDirLine_->setText(imageDir_);
    but1= new QPushButton(tr("Browse"));
    connect(but1,SIGNAL(clicked(bool)),this,SLOT(setLabelImageDirFromBrowse()));

    ImageDirLayout->addWidget(label1);
    ImageDirLayout->addWidget(imageDirLine_);

    ImageDirLayout->addWidget(but1);

    QBoxLayout * LogFileLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    LogFileLayout->setMargin(marginvertical);
    LogFileLayout->setSpacing(spacinghorizontal);

    QLabel * label2= new QLabel(tr("Log file:"));
    logFileLine_ = new QLineEdit;
    logFile_ = storedSet_->value("/Settings/LogFile","").toString();
    logFileLine_->setText(logFile_);
    but2= new QPushButton(tr("Browse"));
    connect(but2,SIGNAL(clicked(bool)),this,SLOT(setLabelLogFileFromBrowse()));

    LogFileLayout->addSpacing(68);
    LogFileLayout->addWidget(label2);
    LogFileLayout->addWidget(logFileLine_);
    LogFileLayout->addWidget(but2);

    QBoxLayout * OutputFileLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    OutputFileLayout->setMargin(marginvertical);
    OutputFileLayout->setSpacing(spacinghorizontal);

    QLabel * label3= new QLabel(tr("Output file:"));
    outputFileLine_ = new QLineEdit;
    outputFile_ = storedSet_->value("/Settings/OutputFile","").toString();
    outputFileLine_->setText(outputFile_);
    but3= new QPushButton(tr("Browse"));
    connect(but3,SIGNAL(clicked(bool)),this,SLOT(setLabelOutputFileFromBrowse()));

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

    QLabel * flabel= new QLabel(tr("Period (s):"));
    freqBox_ = new QSpinBox;
    frequency_=storedSet_->value("/Settings/Frequency",15).toInt();
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
    //BLOCK PARENT
    this->show();
}

void SettingsWindow::setLabelImageDirFromBrowse(){
    QString str = QFileDialog::getExistingDirectory(0,"Choose the directory to image files","");
    imageDirLine_->setText(str);
}


void SettingsWindow::setLabelLogFileFromBrowse(){
    QString str = QFileDialog::getOpenFileName(0,"Choose the path to the log file","","*.txt");
    logFileLine_->setText(str);
}

void SettingsWindow::setLabelOutputFileFromBrowse(){
    QString str = QFileDialog::getOpenFileName(0,"Choose the path to the output file","","*.txt");
    outputFileLine_->setText(str);
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
        storedSet_->setValue("/Settings/ImageDir",imageDir_);
    }

    QString logFileTemp= logFileLine_->text();

    //check file specification

    if(!QFile(logFileTemp).exists()){
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the log-file"),QMessageBox::Ok);
        allisok=false;
    }
    else{
        logFile_=logFileTemp;
        storedSet_->setValue("/Settings/LogFile",logFile_);
    }

    QString  outputFileTemp = outputFileLine_->text();
    if(!QFile(outputFileTemp).exists()){
        QMessageBox::critical(this,tr("Attention"),tr("Incorrect path to the output file"),QMessageBox::Ok);
        allisok=false;
    }
    else{
        outputFile_=outputFileLine_->text();
        storedSet_->setValue("/Settings/OutputFile",outputFile_);
    }

    frequency_=freqBox_->value();
    storedSet_->setValue("Settings/Frequency",frequency_);

    if(allisok){
        emit sendSettingsToProcessor(imageDir_,logFile_,outputFile_,frequency_);
        this->hide();
    }
}

void SettingsWindow::cancelSettings(){
    //Are QLineEdit being deleted themself?
    this->hide();
}

void SettingsWindow::changeState(int state){
    //only paused state
    if(state==1){
        QPalette  palette;
        palette.setColor(QPalette::Base,Qt::gray);
        palette.setColor(QPalette::Text,Qt::darkGray);

        imageDirLine_->setReadOnly(true);
        imageDirLine_->setPalette(palette);

        logFileLine_->setReadOnly(true);
        logFileLine_->setPalette(palette);

        outputFileLine_->setReadOnly(true);
        outputFileLine_->setPalette(palette);

        but1->setDisabled(true);
        but2->setDisabled(true);
        but3->setDisabled(true);
    }
}

StatBar::StatBar(QWidget *parent):QLabel(parent),stateArr_({tr("Stopped") , tr("Paused"), tr("Working")}){
    this->setText(stateArr_.at(0)); //initialize by "stopped" value
}

void StatBar::changeState(int s){
    this->setText(stateArr_.at(s));
    this->repaint();
}


PausePlayButton::PausePlayButton(QWidget *parent):QPushButton(parent),state_(0),IconsPath_({":/img/on.png",":/img/pause.png"}){
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
    state_=!state_;
    emit pausePlayButtonClicked(state_+1);
    //inner state =0 (paused) -> external state = 1
    //inner state =1 (working) -> external state = 2
}

void PausePlayButton::switchView(int state){
    //input parameter state in external format
    if (state ==0 || state ==1){
        state_=0;
    }
    else {
        state_=1;
    }
    this->setIcon(*ButtonIcon_[state_]);
    this->setIconSize(ButtonPicture_[state_]->rect().size());
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
    if (state==0 || state ==1){
        this->setDisabled(false);
    }
    else{
        this->setDisabled(true);
    }
}
