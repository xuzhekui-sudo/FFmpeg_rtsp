#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "xvideothread.h"
#include "QMessageBox"
#include "QString"

//QString url="rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4";
QString url="rtmp://ns8.indexforce.com/home/mystream";
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineEdit_url->setText(url);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//开始连接
void MainWindow::on_pushButton_link_clicked()
{
    QString url=ui->lineEdit_url->text();
    if(url.isEmpty())
    {
        QMessageBox::warning(this,"警告","当前输入地址为空");
    }
    else
    {
        vt = new xVideoThread(url);
        connect(vt, &xVideoThread::updateVideo, this, &MainWindow::VideoUpateHandle);
        connect(vt, &xVideoThread::updateMsg, this, &MainWindow::MsgUpateHandle);
        vt->start();
    }
}
void MainWindow::VideoUpateHandle(const QImage& img)
{
    ui->label_video->Repaint(img);
}

void MainWindow::MsgUpateHandle()
{
    QMessageBox::warning(nullptr,"warning","Device initialization failed");
}
