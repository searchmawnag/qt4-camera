#include "videodevice.h"
#include <QDebug>

VideoDevice::VideoDevice(QString dev_name)
{
    this->dev_name = dev_name;
    this->fd = -1;
    this->buffers = NULL;
    this->numBufs = 0;
    this->index = -1;
    this->m_isopened = false;
    this->m_isstarted = false;
}

VideoDevice::VideoDevice()
{
    this->fd = -1;
    this->buffers = NULL;
    this->numBufs = 0;
    this->index = -1;
    this->m_isopened = false;
    this->m_isstarted = false;
}

void VideoDevice::setDeviceName(QString dev_name)
{
    this->dev_name = dev_name;
}

int VideoDevice::open_device()
{
    fd = open(dev_name.toStdString().c_str(), O_RDWR, 0);

    if(-1 == fd)
    {
        emit display_error(tr("open: %1").arg(QString(strerror(errno))));
        return -1;
    }
    m_isopened = true;
    return 0;
}

int VideoDevice::close_device()
{
    if(isOpen()){
        if(-1 == close(fd))
        {
            emit display_error(tr("close: %1").arg(QString(strerror(errno))));
            return -1;
        }
        m_isopened = false;
    }
    return 0;
}

int VideoDevice::init_device()
{
    struct  v4l2_capability cap;
//    struct  v4l2_cropcap cropcap;
//    struct  v4l2_crop crop;

    if(-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        if(EINVAL == errno)
        {
            emit display_error(tr("%1 is no V4l2 device").arg(dev_name));
        }
        else
        {
            emit display_error(tr("VIDIOC_QUERYCAP: %1").arg(QString(strerror(errno))));
        }
        return -1;
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        emit display_error(tr("%1 is no video capture device").arg(dev_name));
        return -1;
    }

    if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        emit display_error(tr("%1 does not support streaming i/o").arg(dev_name));
        return -1;
    }

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 320;
    fmt.fmt.pix.height = 240;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
//    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if(-1 == ioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        emit display_error(tr("VIDIOC_S_FMT").arg(QString(strerror(errno))));
        return -1;
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(fd, VIDIOC_G_FMT, &fmt)){
        emit display_error(tr("VIDIOC_G_FMT").arg(QString(strerror(errno))));
        return -1;
    }
    qDebug() << "Width = " << fmt.fmt.pix.width << "Height = " << fmt.fmt.pix.height;

    struct v4l2_streamparm setfps;
    CLEAR(setfps);
    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(fd, VIDIOC_G_PARM, &setfps)){
        emit display_error(tr("VIDIOC_G_PARM").arg(QString(strerror(errno))));
        return -1;
    }
    setfps.parm.capture.timeperframe.numerator = 1;
    setfps.parm.capture.timeperframe.denominator = 30;
    if(-1 == ioctl(fd, VIDIOC_S_PARM, &setfps)){
        emit display_error(tr("VIDIOC_S_PARM").arg(QString(strerror(errno))));
        return -1;
    }

    if(-1 == init_mmap())
    {
        return -1;
    }

    return 0;
}

int VideoDevice::init_mmap()
{
    struct  v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        if(EINVAL == errno)
        {
            emit display_error(tr("%1 does not support memory mapping").arg(dev_name));
            return -1;
        }
        else
        {
            emit display_error(tr("VIDIOC_REQBUFS %1").arg(QString(strerror(errno))));
            return -1;
        }
    }

    buffers = (Videobuffer*)calloc(req.count, sizeof(Videobuffer));

    if(!buffers)
    {
        emit display_error(tr("out of memory"));
        return -1;
    }

    struct  v4l2_buffer buf;
    for(numBufs = 0; numBufs < req.count; ++numBufs)
    {
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = numBufs;

        if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            emit display_error(tr("VIDIOC_QUERYBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }

        buffers[numBufs].length = buf.length;
        buffers[numBufs].offset = (size_t)buf.m.offset;
        buffers[numBufs].start =
                (uchar*)mmap(NULL, // start anywhere
                     buf.length,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     fd, buf.m.offset);

        if(MAP_FAILED == buffers[numBufs].start)
        {
            emit display_error(tr("mmap %1").arg(QString(strerror(errno))));
            return -1;
        }
    }
    return 0;

}

int VideoDevice::start_capturing()
{
    unsigned int i;
    for(i = 0; i < numBufs; ++i)
    {
        struct  v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory =V4L2_MEMORY_MMAP;
        buf.index = i;
//        fprintf(stderr, "n_buffers: %d\n", i);

        if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            emit display_error(tr("VIDIOC_QBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(-1 == ioctl(fd, VIDIOC_STREAMON, &type))
    {
        emit display_error(tr("VIDIOC_STREAMON: %1").arg(QString(strerror(errno))));
        return -1;
    }
    m_isstarted = true;
    return 0;
}

int VideoDevice::stop_capturing()
{
    v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    m_isstarted = false;
    if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        emit display_error(tr("VIDIOC_STREAMOFF: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}

int VideoDevice::uninit_device()
{
    unsigned int i;
    for(i = 0; i < numBufs; ++i)
    {
        if(-1 == munmap(buffers[i].start, buffers[i].length))
        {
            emit display_error(tr("munmap: %1").arg(QString(strerror(errno))));
            return -1;
        }

    }
    free(buffers);
    return 0;
}

int VideoDevice::get_frame(void **frame_buf, size_t* len)
{
    struct  v4l2_buffer buf;
    uchar* ptcur;
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
    {
        switch(errno)
        {
        case EAGAIN:
            perror("dqbuf");
            return -1;
        case EIO:
            return -1 ;
        default:
            emit display_error(tr("VIDIOC_DQBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }

    index = buf.index;
    ptcur = buffers[0].start;

    *frame_buf = ptcur;
    *len = buf.bytesused;

    return 0;
}

int VideoDevice::unget_frame()
{
    if(index != -1)
    {
        struct  v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;

        if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            emit display_error(tr("VIDIOC_QBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
        return 0;
    }
    return -1;
}
