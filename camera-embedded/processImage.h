#ifndef PROCESSIMAGE_H
#define PROCESSIMAGE_H

#include <QDialog>
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>
#include "videodevice.h"

namespace Ui {
    class Dialog;
}
class QTimer;

class ProcessImage : public QDialog
{
    Q_OBJECT
public:
    ProcessImage(QWidget *parent=0);
    void initCamera();
    ~ProcessImage();

private:
//    int convert_yuv_to_rgb_pixel(int y, int u, int v);
//    int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
//    unsigned short RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue);

private slots:
    void paintEvent(QPaintEvent *);
    void display_error(QString err);
    void repaintFrame();

    void setnet();
    void connectTCP();
    void closeTCP();

private:
    VideoDevice *vd;
//    QPainter *painter;
//    QLabel *label;
    QImage *frame;
    //QPixmap *frame;
    QByteArray frame_buf;
    QTimer *timer;
    int rs;
    uchar *frame_rgb;
    uchar * frame_yuv;
    unsigned int len;
    bool hav_video;

    QTcpSocket m_client;
    QHostAddress ip;
    quint16 port;

    Ui::Dialog *ui;
};

#endif
