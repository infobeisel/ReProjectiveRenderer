#include "canonicalglwindowimpl.h"
#include <QGuiApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    QSurfaceFormat format;
    format.setVersion(4,1); //core version 410
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples( 4 );
    format.setDepthBufferSize( 24 );
    CanonicalGLWindowImpl *modelWindow = 0;
    modelWindow = new CanonicalGLWindowImpl();
    modelWindow->setFormat(format);
    modelWindow->resize(1024,512);
    modelWindow->show();
    return a.exec();
}
