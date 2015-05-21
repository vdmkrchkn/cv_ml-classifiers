#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "ml.h"

enum ClsMethod
{
    mSVM,
    BAYES
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_action_Open_triggered();
    void on_action_Create_triggered();
    void on_action_Train_triggered();
    void on_action_Recognize_triggered();
    void on_action_Test_triggered();
private:
    Ui::MainWindow *ui;
    QLabel *m_pLabelAccuracy;
    // метка для отрисовки изображения
    QLabel *m_pLabelImage;
    QImage m_pImage;
    // метка для размера файла
    QLabel *m_pLabelImageSize;    
    //
    ClsMethod clMethod;
    CvStatModel *cv_model;
    //
    enum
    {                
        nSignalSz = 2500,     // размер входного сигнала
        nTrainingSamples = 16,// размер обучающей выборки
        nTestSamples = 22     // размер тестовой выборки
    };
    //
    void updateStatusBar();
};

#endif // MAINWINDOW_H
