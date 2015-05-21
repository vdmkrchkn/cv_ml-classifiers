#ifndef TRAINDIALOG_H
#define TRAINDIALOG_H

#include <QDialog>
#include "ui_train-dialog.h"

class TrainDialog : public QDialog, public Ui::TrainDialog
{
    Q_OBJECT

public:
    explicit TrainDialog(QWidget *parent = 0);    
};

#endif // TRAINDIALOG_H
