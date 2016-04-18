#include <QApplication>
#include <QTextCodec>
#include "processImage.h"
#include <QDebug>

int main(int argc,char *argv[])
{
    QApplication app(argc,argv);

    QTextCodec *gcodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(gcodec);
    QTextCodec::setCodecForCStrings(gcodec);

    ProcessImage process;
    process.show();

    return app.exec();
}
