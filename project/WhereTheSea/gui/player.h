#ifndef PLAYER_H
#define PLAYER_H

#include <QStatusBar>
#include <QMainWindow>
#include <QtWidgets>

#include "dataprocessor.h"

class SettingsWindow;
class StatBar;
class PausePlayButton;
class StopButton;
class SettingsMenu;
class DataProcessor;
class ThreadImagePerform;

class Player: public QMainWindow  {
    Q_OBJECT
private:
    SettingsMenu * settingsMenu_;
    //QMenu * helpMenu_;

    QAction * toSettings_;
    SettingsWindow * setWin_;

    PausePlayButton * buttonOnPause_;
    StopButton * buttonStop_;

    QBoxLayout * pbxLayout_;
    QWidget * widgetForLayout_;

    StatBar * stBar_;

    QSettings * storedSet_;

    ThreadImagePerform * ImThread_;
    DataProcessor * processor_;
public:
    Player (QWidget * parent = nullptr);
    ~Player();
public slots:
    void showMessage(int);
};

class SettingsWindow: public QDialog{
    Q_OBJECT
private:
    QString imageDir_;
    //QString logFile_;
    int logFile_; // 0 -unchecked ,2 -checked
    QString outputFile_;
    int minFile_;
    int scale_;


    QLineEdit * imageDirLine_;
    //QLineEdit * logFileLine_;
    QCheckBox * logFileBox_;
    QLineEdit * outputFileLine_;
    QSpinBox * minFileBox_;
    QPushButton * but1;
    QPushButton * but2;
    QSpinBox * scaleBox_;

    QSettings * storedSet_;
    friend class ::Player;
public:
    SettingsWindow(QSettings * storedSet, QWidget * parent =nullptr);

    void checkInput();
signals:
    void sendSettingsToProcessor(QString imDir, int logFile, QString outputFile, int minFile,int scale);
public slots:
    void openSettingsWindow(); // open settings window on pressing the button in menus

    void setLabelImageDirFromBrowse();
    void setLabelOutputFileFromBrowse();

    void applySetings();
    void cancelSettings();

     void changeState(int); // desable while "paused"
};

class StatBar: public QLabel{
    Q_OBJECT
private:
    const QVector<QString> stateArr_;
public:
    StatBar(QWidget * parent = nullptr);
public slots:
    void changeState(int);
};

class PausePlayButton: public QPushButton{
    Q_OBJECT
private:
    QVector<QString> IconsPath_;
    QVector<QPixmap *> ButtonPicture_;
    QVector<QIcon *> ButtonIcon_;

    int state_; //0 - pause, 1- on
public:
    PausePlayButton(QWidget * parent = nullptr);

signals:
    void pausePlayButtonClicked(int);

public slots:
    void switchView(int); //change external view
    void informAll(bool); //change logic state
};

class StopButton: public QPushButton{
    Q_OBJECT
private:
    //const QString IconPath;
    QString IconPath_;
    QPixmap * ButtonPicture_;
    QIcon * ButtonIcon_;
public:
    StopButton(QWidget * parent = nullptr);

signals:
    void stopButtonClicked(int);

public slots:
    void informAll(bool);
};

class SettingsMenu: public QMenu{
    Q_OBJECT
public:
    SettingsMenu(QWidget * parent =nullptr):QMenu(parent){}
    SettingsMenu(const QString & title, QWidget * parent = nullptr): QMenu(title, parent){}
public slots:
    void changeState(int);
};

class ThreadImagePerform:public QThread{
public:
    void run(){
        exec();
    }
};

#endif // PLAYER_H
