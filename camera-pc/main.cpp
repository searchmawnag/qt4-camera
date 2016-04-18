#include "cameradialog.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *gcodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(gcodec);
    QTextCodec::setCodecForCStrings(gcodec);
    QTextCodec::setCodecForTr(gcodec);

    CameraDialog w;
    w.show();

    return a.exec();
}
