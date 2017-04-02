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

class Player: public QMainWindow  {
    Q_OBJECT
private:
    SettingsMenu * settingsMenu_;
    QMenu * helpMenu_;

    QAction * toSettings_;
    SettingsWindow * setWin_;

    PausePlayButton * buttonOnPause_;
    StopButton * buttonStop_;

    QBoxLayout * pbxLayout_;
    QWidget * widgetForLayout_;

    StatBar * stBar_;

    DataProcessor * processor_;
public:
    Player (QWidget * parent = nullptr);

public slots:

};

class SettingsWindow: public QDialog{
    Q_OBJECT
private:
    QString imageDir_;
    QString logFile_;
    QString outputFile_;
    int frequency_;

    QLineEdit * imageDirLine_;
    QLineEdit * logFileLine_;
    QLineEdit * outputFileLine_;
    QSpinBox * freqBox_;
protected:
    int isFileSpecification(const QString &);
public:
    SettingsWindow (QString imDir,QString logFile,QString outputFile, int frequency, QWidget * parent = nullptr);

    void checkInput();
signals:
    void sendSettingsToProcessor(QString imDir, QString logFile, QString outputFile, int frequency);
public slots:
    void openSettingsWindow(); // open settings window on pressing the button in menus
    void applySetings();
    void cancelSettings();
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
    //const QVector<QString> IconsPath;
    QVector<QString> IconsPath_;
    QVector<QPixmap *> ButtonPicture_;
    QVector<QIcon *> ButtonIcon_;

    int state; //0 - pause, 1- on
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

/*class DirValidator: public QValidator{
public:
    DirValidator(QObject * parent = nullptr): QValidator(parent){

    }

};

class FileValifdator: public QValidator{

};*/

#endif // PLAYER_H
