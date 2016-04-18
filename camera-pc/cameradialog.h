#ifndef CAMERADIALOG_H
#define CAMERADIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QLabel>

namespace Ui {
    class CameraDialog;
}

class CameraDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraDialog(QWidget *parent = 0);
    ~CameraDialog();

private slots:
    void newConnection();
    void newData();
    void close(QObject* obj);

    void clientsend();
    void paintEvent(QPaintEvent *);

    void setnet();
    void listenTCP();
    void closeTCP();

private:
    Ui::CameraDialog *ui;
    QTcpServer m_server;
    QTcpSocket m_client;
    QPainter *painter;
    QLabel *label;
    QImage *frame;
    uchar * frame_yuv;
    uchar * p_yuv;
    bool hav_video;
    QTimer m_tim;


    QHostAddress ip;
    quint16 port;
};

#endif // CAMERADIALOG_H
