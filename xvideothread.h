#ifndef XVIDEOTHREAD_H
#define XVIDEOTHREAD_H

#include <QObject>
#include <QThread>
#include <xdecode.h>

class xVideoThread:public QThread
{
    Q_OBJECT
public:
    explicit xVideoThread(QString url,QObject *parent = 0);
    virtual void close();
    void run();
signals:
    void updateVideo(const QImage&);
    void updateMsg();

private:
    xDecode *m_decode;
    QImage m_image;
    QString m_Url;
};

#endif // XVIDEOTHREAD_H
