#include "cameradialog.h"
#include "ui_cameradialog.h"
#include <QString>
#include <QByteArray>
#include <QMessageBox>

#define MAXBUFFER 1024*1024

CameraDialog::CameraDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraDialog)
{
    ui->setupUi(this);
    frame = new QImage(320,240,QImage::Format_RGB888);
    frame_yuv = new uchar[320*240*3];
    p_yuv = frame_yuv;
//    label = new QLabel();
    hav_video = false;

    QString addr("192.168.1.52");
    ip.setAddress(addr);
    port = 8888;
    ui->lineEdit_ip->setText(addr);
    ui->lineEdit_port->setText(QString::number(port));
//    listenTCP();

    connect(ui->pushButton_set, SIGNAL(clicked()), this, SLOT(setnet()));
    connect(ui->pushButton_listen, SIGNAL(clicked()), this, SLOT(listenTCP()));
    connect(ui->pushButton_close, SIGNAL(clicked()), this, SLOT(closeTCP()));

//    connect(&m_client, SIGNAL(disconnected()), this, SLOT(closed()));
//    m_client.connectToHost(ip, port);

//    connect(&m_tim, SIGNAL(timeout()), this, SLOT(clientsend()));
//    m_tim.start(3000);
}

CameraDialog::~CameraDialog()
{
    delete ui;
}

void CameraDialog::newConnection()
{
    QTcpSocket* client = m_server.nextPendingConnection();
    qDebug()<< "newConnection" << client;
    while (client)
    {
//		Conn* conn = new Conn;
//		if (!conn)
//		{
//			client->deleteLater();
//		}
//		else
//		{
//			client->setProperty(PROP_CONN, qVariantFromValue((void*)conn));

//			conn->client = client;
//			conn->key = TK::ipstr(client->peerAddress(),client->peerPort(), true);

            connect(client, SIGNAL(readyRead()), this, SLOT(newData()));
//            connect(client, SIGNAL(destroyed(QObject*)), this, SLOT(close(QObject*)));
            connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));
//			connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error()));

//			setCookie(conn->key, conn);
//		}
        client = m_server.nextPendingConnection();
    }
}

void CameraDialog::newData()
{
    static QByteArray buf0;
    static bool isBegain = false;
    static uint frame_len =0;
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    qint64 bufLen = client->bytesAvailable();
    char *buf = new char[bufLen+1];
    if (!buf) return;
//qDebug() << "bytesAvailable: " << bufLen;
    qint64 readLen = client->read(buf, bufLen);
    qDebug() << "readLen" << readLen/* <<":" << buf*/;
    buf0.append(buf, bufLen);
    if(!isBegain){
        p_yuv = frame_yuv;
        frame_len = *(unsigned int*)buf0.data();qDebug() << "DataLen: " << frame_len;
//        frame_len = *(unsigned int*)buf;qDebug() << "datalen: " << frame_len;
        buf0.remove(0, 4);
//        memcpy(p_yuv, buf+sizeof(frame_len), readLen - sizeof(frame_len));

        isBegain = true;
    }
    else {
//        uint len;
//        if((p_yuv - frame_yuv + readLen) >= frame_len){
//            len = frame_len - (p_yuv - frame_yuv);
//            isBegain = false;
//            hav_video = true;
//            this->update();
//        }
//        else{
//            len = readLen;
//        }
//        memcpy(p_yuv, buf, len);
//        p_yuv += len;

        if(buf0.length() >= frame_len){
            memcpy(frame_yuv, buf0.left(frame_len).data(), frame_len);
            buf0.remove(0, frame_len);
            isBegain = false;
            hav_video = true;
            this->update();
        }
    }

    if(readLen > 16){
//        hav_video = true;
//        this->update();
    }
}

void CameraDialog::close(QObject *obj)
{
    qDebug() << "TCP Closed.";
}

void CameraDialog::clientsend()
{
    m_client.write("client test data");
}

void CameraDialog::paintEvent(QPaintEvent *)
{
    if(hav_video){
        frame->loadFromData((uchar *)frame_yuv,/*len*/320 * 240 * 3 * sizeof(char));

//        label->setPixmap(QPixmap::fromImage(*frame,Qt::AutoColor));
        ui->label_video->setPixmap(QPixmap::fromImage(*frame,Qt::AutoColor));
        hav_video = false;

    }
}

void CameraDialog::setnet()
{
    QString addr(ui->lineEdit_ip->text());
    ip.setAddress(addr);
    port = ui->lineEdit_port->text().toUInt();
}

void CameraDialog::listenTCP()
{
    if(m_server.isListening()){
        closeTCP();
    }
    if(m_server.listen(ip, port)){
        connect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
        ui->pushButton_listen->setEnabled(false);
        qDebug() << "listen...";
    }
    else{
        QMessageBox::warning(this, QString("Error!!!"), QString("填写本机IP，或者更换端口号后重试！！！"));
    }
}

void CameraDialog::closeTCP()
{
    qDebug() << "closetcp";
//    if(m_server.isListening()){
        disconnect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
        m_server.close();
        ui->pushButton_listen->setEnabled(true);
        qDebug() << "close...";
//    }
}
