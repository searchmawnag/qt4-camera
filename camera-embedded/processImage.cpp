#include "ui_dialog.h"
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QByteArray>
#include "processImage.h"
#include "videodevice.h"

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
}
ProcessImage::ProcessImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    setWindowTitle(tr("Capture"));
    ui->setupUi(this);
    timer = new QTimer(this);
    frame = new QImage(320,240,QImage::Format_RGB888);
    hav_video = false;

    initCamera();

    QString addr("192.168.43.2");
    ip.setAddress(addr);
    port = 8888;
    ui->lineEdit_ip->setText(addr);
    ui->lineEdit_port->setText(QString::number(port));
    connectTCP();

    connect(timer,SIGNAL(timeout()),this,SLOT(repaintFrame()));
    timer->start(100);

    connect(ui->pushButton_listen, SIGNAL(clicked()), this, SLOT(connectTCP()));
    connect(ui->pushButton_close, SIGNAL(clicked()), this, SLOT(closeTCP()));
}

ProcessImage::~ProcessImage()
{
    if(vd->isStart()){
        rs = vd->stop_capturing();
        rs = vd->uninit_device();
        rs = vd->close_device();
    }
}

void ProcessImage::initCamera()
{
    vd = new VideoDevice(QString("/dev/video0"));

    connect(vd, SIGNAL(display_error(QString)), this,SLOT(display_error(QString)));

    rs = vd->open_device();
    if(-1==rs)
    {
        return;
    }

    rs = vd->init_device();
    if(-1==rs)
    {
        vd->close_device();
        return;
    }

    rs = vd->start_capturing();
    if(-1==rs)
    {
        vd->close_device();
        return;
    }

}

void ProcessImage::paintEvent(QPaintEvent *)
{
    if(hav_video){
        ui->label_video->setPixmap(QPixmap::fromImage(*frame,Qt::AutoColor));
        hav_video = false;
    }
}

void ProcessImage::display_error(QString err)
{
    qDebug() << "ERR: " << err;
}

void ProcessImage::repaintFrame()
{
    if(!vd->isOpen() || !vd->isStart()){
        initCamera();
        return;
    }

    rs = vd->get_frame((void **)&frame_yuv,&len);
    if(-1==rs)
    {
        timer->setInterval(3000);
        vd->stop_capturing();
        vd->close_device();
        initCamera();
        return;
     }
    frame_buf.setRawData((const char*)frame_yuv, len);
    vd->unget_frame();
    frame->loadFromData((uchar *)frame_buf.data(),len/*320 * 240 * 3 * sizeof(char)*/);

    if(m_client.isValid()){
        rs = m_client.write((const char *)&len, sizeof(len));
        m_client.flush();
        if(rs < 0){
            closeTCP();
            connectTCP();
        }
        rs = m_client.write((const char *)frame_buf.data(), len);
        m_client.flush();qDebug() << rs << "";
        if(rs < 0){
            closeTCP();
            connectTCP();
        }
    }
    else{
        closeTCP();
        connectTCP();
    }
    hav_video = true;
    this->update();
}

void ProcessImage::setnet()
{
    QString addr(ui->lineEdit_ip->text());
    ip.setAddress(addr);
    port = ui->lineEdit_port->text().toUInt();
}

void ProcessImage::connectTCP()
{
    qDebug() << "connectToHost" << "ip:" << ip << "port:" << port;
    m_client.connectToHost(ip,port);
    ui->pushButton_listen->setEnabled(false);
    if(!m_client.isValid()){
        QMessageBox::warning(this, QString("Error!!!"), QString("填写本机IP，或者更换端口号后重试！！！"));
    }
}

void ProcessImage::closeTCP()
{
    qDebug() << "close connect";
    m_client.disconnectFromHost();
    ui->pushButton_listen->setEnabled(true);
    qDebug() << "close...";
}
