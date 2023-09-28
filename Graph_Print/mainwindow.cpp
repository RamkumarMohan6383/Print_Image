#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <iostream>
#include "printer.h"
#include <QDebug>
#include <unistd.h>
#include <QPainter>
#include <QBuffer>
#include <QPainter>
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

QImage MainWindow::loadImage(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull())
    {
        qDebug() << "Error loading image: " << filePath;
    }
    return image;
}

void MainWindow::on_pushButton_clicked()
{
    QApplication::processEvents();

    Printer *printer = new Printer();
    std::cout << "Trying to open port..." << std::endl;
    bool res = printer->open("/dev/ttyS0");
    std::cout << "Status: " << res << std::endl;
    if (!res)
    {
        std::cerr << "Error opening port, aborting" << std::endl;
        return;
    }
    QString imagePath = "/home/pi/git/Graph_Print/Raspberry_Pi.jpg"; // Replace with the path to your image

    QImage image = loadImage(imagePath);

    if (!image.isNull())
    {
        qDebug()<<"Image enter";

        printer->write("Data");
        printer->feed();
        printer->feed();
        printer->printImage(image);
        printer->feed();
        printer->feed();

    }
    else
    {
        qDebug() << "Image is null or invalid.";
    }

    printer->close();
}



void MainWindow::on_pushButton_2_clicked()
{
    
}
