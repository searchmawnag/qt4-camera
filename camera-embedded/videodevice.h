#ifndef VIDEODEVICE_H
#define VIDEODEVICE_H

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#include <sys/ioctl.h>
#include <sys/mman.h>

#include <asm/types.h>
#include <linux/videodev2.h>

#include <QString>
#include <QObject>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

class VideoDevice : public QObject
{
    Q_OBJECT
public:
    VideoDevice(QString dev_name);
    VideoDevice();
    void setDeviceName(QString dev_name);
    int open_device();
    int close_device();
    int init_device();
    int start_capturing();
    int stop_capturing();
    int uninit_device();
    int get_frame(void **, size_t*);
    int unget_frame();
    inline bool isOpen(){return m_isopened;}
    inline bool isStart(){return m_isstarted;}

private:
    int init_mmap();

    struct Videobuffer
    {
        uchar*  start;
        size_t  offset;
        size_t  length;
    };
    QString dev_name;
    int fd;
    struct  Videobuffer* buffers;
    struct  v4l2_format fmt;
    unsigned int numBufs;
    int index;
    bool m_isopened;
    bool m_isstarted;

signals:
    void display_error(QString);

};

#endif // VIDEODEVICE_H
