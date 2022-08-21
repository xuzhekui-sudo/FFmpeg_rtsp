#include "xvideothread.h"
#include "QMessageBox"
xVideoThread::xVideoThread(QString url,QObject *parent):QThread(parent)
{
    m_Url = url;
}

void xVideoThread::close()
{

}

void xVideoThread::run()
{
    m_decode = new xDecode(m_Url);
    if(nullptr==m_decode)
        return;
   if(false== m_decode->begin())
   {
        emit updateMsg();
        return;
   }
    while(1)
    {
       if(true == m_decode->samp(m_image))
       {
            emit updateVideo(m_image);
            msleep(1);
       }
       else
       {
           break;
       }
    }
    m_decode->end();
    delete m_decode;
}
