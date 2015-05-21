#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "train-dialog.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include <iostream>
#include <stdexcept>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace
{    
    // преобразование 2-мерного массива в 1-мерный
    void convertMat(const Mat& srcMat, Mat& dstMat)
    {        
        for(int i = 0; i < srcMat.rows; ++i)
            for(int j = 0; j < srcMat.cols; ++j)
                dstMat.at<float>(i * srcMat.cols + j) = srcMat.at<int>(i,j);
    }
    // чтение и разметка образцов
    void readSamples(const QString& dirPath, const int nSamples, Mat& images, Mat& classes)
    {
        QDir imgDir(dirPath);
        QFileInfoList files = imgDir.entryInfoList(QStringList("*.jpg"));
        if(nSamples != files.size())
            throw std::invalid_argument("invalid sample size");
        for(int j = 0; j < nSamples; ++j)
        {
            QString fName = files[j].fileName();
            int type = fName[fName.indexOf(QChar('-')) - 1].digitValue();
            if(type != -1)
            {
                Mat img;
                imread(imgDir.absoluteFilePath(fName).toStdString(),CV_LOAD_IMAGE_GRAYSCALE)
                        .convertTo(img,CV_32S,1 / 255.0, -1);
                Mat vec(1,img.rows * img.cols,images.type());
                convertMat(abs(img),vec);
                images.push_back(vec);
                classes.at<float>(j) = type;// == 0 ? -1 : 1;
            }
        }
    }
    // подсчёт правильно распознанных классов
    float evaluate(const Mat& actual, const Mat& predicted)
    {
        int t = 0, f = 0;
        for(int i = 0; i < actual.rows; ++i)
        {            
            if(actual.at<float>(i) == predicted.at<float>(i))
                ++t;
            else
                ++f;
        }
        return t * 1.0 / (t + f);
    }        
    // вывод 2-мерного массива на экран
    void printMat(const Mat& matr)
    {
        for(int i = 0; i < matr.rows; ++i)
        {
            for(int j = 0; j < matr.cols; ++j)
                 std::cout << matr.at<float>(i,j) << " ";
            std::cout << std::endl;
        }
    }
    // выделение имени файла из абсолютного пути
    QString strippedName(const QString &rcFullFileName)
    {
        return QFileInfo(rcFullFileName).fileName();
    }    
}
//
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //
    m_pLabelImage = new QLabel;
    m_pLabelImage->setAlignment(Qt::AlignCenter);
    setCentralWidget(m_pLabelImage);
    //
    m_pLabelAccuracy = new QLabel;
    statusBar()->addWidget(m_pLabelAccuracy, 1);
    //    
    m_pLabelImageSize = new QLabel("9999 x 9999");
    m_pLabelImageSize->setAlignment(Qt::AlignHCenter);
    m_pLabelImageSize->setMinimumSize(m_pLabelImageSize->sizeHint());
    //    
    statusBar()->addWidget(m_pLabelImageSize);
    printMat(Mat());
    updateStatusBar();        
}
//
MainWindow::~MainWindow()
{
    delete m_pLabelImage;    
    delete m_pLabelImageSize;
    delete m_pLabelAccuracy;
    delete ui;
    delete cv_model;
}
//
void MainWindow::on_action_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"),
                                                    "images/test",
                                                    tr("JPEG images (*.jpg)"));
    if (!fileName.isEmpty())
    {
        QPixmap pixmap;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const bool cbSuccess = pixmap.load(fileName);
        QApplication::restoreOverrideCursor();
        //
        if(cbSuccess)
        {
            m_pLabelImage->setPixmap(pixmap);
            m_pImage = pixmap.toImage();            
            setWindowTitle(strippedName(fileName));
            updateStatusBar();
            statusBar()->showMessage(tr("File loaded"), 2000);
        }
    }
}
//
void MainWindow::on_action_Create_triggered(){}
//
void MainWindow::on_action_Train_triggered()
{
    TrainDialog* tDialog = new TrainDialog(this);
    if(tDialog->exec())
    {
        clMethod = (ClsMethod)tDialog->comboBox->currentIndex();
        // stub for BAYES
        if(clMethod == BAYES)
        {
            cv_model = new CvNormalBayesClassifier();
            return;
        }
        else
            cv_model = new CvSVM();
        // training set preparation
        Mat trainingData(0,nSignalSz,CV_32F);
        Mat trainingClasses(nTrainingSamples,1,CV_32F);
        try{
            readSamples("images/train",nTrainingSamples,trainingData,trainingClasses);
        }catch(const std::invalid_argument& ia){
            std::cerr << "Error in \"on_action_Train_triggered\": " << ia.what() << std::endl;
            return;
        }
        if(clMethod == mSVM)
        {
            CvSVMParams params = CvSVMParams();
            //  n-class classification with imperfect separation of classes with penalty
            //  multiplier C for outliers
            params.svm_type = CvSVM::C_SVC;
            params.kernel_type = CvSVM::LINEAR;
            params.degree = params.gamma = params.coef0 = params.nu = params.p = 0;
            params.C = 7;    // penalty multiplier
            params.class_weights = NULL; // for C_SVC
            params.term_crit.type = CV_TERMCRIT_ITER | CV_TERMCRIT_EPS;
            params.term_crit.max_iter = tDialog->nIterSpinBox->value();
            params.term_crit.epsilon = pow(10,-tDialog->eSpinBox->value());  //1e-6            
            dynamic_cast<CvSVM*>(cv_model)->train(trainingData,trainingClasses,Mat(),Mat(),params);
        }
        else
            dynamic_cast<CvNormalBayesClassifier*>(cv_model)->train(trainingData,trainingClasses);
        //
        ui->action_Test->setEnabled(true);
        ui->action_Recognize->setEnabled(true);
    }
    delete tDialog;
}
//
void MainWindow::on_action_Recognize_triggered()
{
    if(m_pImage.isNull())
        QMessageBox::warning(this,tr("Warning"),tr("No image to process"));            
    else
    {
        Mat mImg(m_pImage.height(),m_pImage.width(),CV_32F);
        // преобразование к значениям {0,1}
        QRgb pb;
        for(int x = 0; x < m_pImage.width(); ++x)
            for(int y = 0; y < m_pImage.height(); ++y)
            {
                pb = m_pImage.pixel(x,y);
                mImg.at<float>(y,x) = abs(round(qRed(pb) / 255.0) - 1);
            }
        //
        Mat vImg(1,mImg.rows * mImg.cols,mImg.type());
        convertMat(mImg,vImg);
        m_pLabelAccuracy->setText("Class = " + QString::number(dynamic_cast<CvSVM*>(cv_model)->predict(vImg)));
    }
}
//
void MainWindow::on_action_Test_triggered()
{
    // test set preparation    
    Mat testData(0,nSignalSz,CV_32F);
    Mat testClasses(nTestSamples,1,CV_32F);
    try{
        readSamples("images/test",nTestSamples,testData,testClasses);
    }catch(const std::invalid_argument& ia){
        std::cerr << "Error in \"on_action_Test_triggered\": " << ia.what() << std::endl;
        return;
    }
    // prediction
    Mat predicted(testClasses.rows, 1, CV_32F);
    if(this->clMethod == mSVM)
    {
        CvSVM *cv_svm = dynamic_cast<CvSVM*>(cv_model);
        for(int i = 0; i < testData.rows; ++i)
        {
            Mat sample = testData.row(i);
            predicted.at<float>(i) = cv_svm->predict(sample);
        }
        //delete cv_svm;
    }
    else
    {
        for(int i = 0; i < testData.rows; ++i)
        {
            Mat sample = testData.row(i);
            predicted.at<float>(i) = dynamic_cast<CvNormalBayesClassifier*>(cv_model)->predict(sample);
        }
    }
    //
    m_pLabelAccuracy->setText("Accuracy = " + QString::number(evaluate(testClasses, predicted)));
//    std::cout << "actual =" << std::endl << testClasses << std::endl;
//    std::cout << "predicted =" << std::endl << predicted << std::endl;
}
//
void MainWindow::updateStatusBar()
{
    QString strLabel;
    if(!m_pImage.isNull()){
        QSize imgSize = m_pImage.size();
        strLabel = QString("%1 x %2")
            .arg(imgSize.width())
            .arg(imgSize.height());
    }
    //    
    m_pLabelImageSize->setText(strLabel);
}
