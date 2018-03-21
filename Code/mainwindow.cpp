#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "math.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- Functions that listen for widget events
// forewards to the mainview

void MainWindow::on_ResetRotationButton_clicked(bool checked)
{
    Q_UNUSED(checked);
    ui->RotationDialX->setValue(0);
    ui->RotationDialY->setValue(0);
    ui->RotationDialZ->setValue(0);
    ui->mainView->setRotation(0, 0, 0);
}

void MainWindow::on_RotationDialX_sliderMoved(int value)
{
    ui->mainView->setRotation(value,
                              ui->RotationDialY->value(),
                              ui->RotationDialZ->value());
}

void MainWindow::on_RotationDialY_sliderMoved(int value)
{
    ui->mainView->setRotation(ui->RotationDialX->value(),
                              value,
                              ui->RotationDialZ->value());
}

void MainWindow::on_RotationDialZ_sliderMoved(int value)
{
    ui->mainView->setRotation(ui->RotationDialX->value(),
                              ui->RotationDialY->value(),
                              value);
}

void MainWindow::on_PhongButton_toggled(bool checked)
{
    if (checked)
    {
        ui->mainView->setShadingMode(MainView::PHONG);
        ui->mainView->update();
    }
}

void MainWindow::on_NormalButton_toggled(bool checked)
{
    if (checked)
    {
        ui->mainView->setShadingMode(MainView::NORMAL);
        ui->mainView->update();
    }
}

void MainWindow::on_GouraudButton_toggled(bool checked)
{
    if (checked)
    {
        ui->mainView->setShadingMode(MainView::GOURAUD);
        ui->mainView->update();
    }
}
