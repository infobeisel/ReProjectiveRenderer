#include "canonicalglwindowimpl.h"
#include <QGuiApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    QSurfaceFormat format;
    format.setVersion(3,3); //core version 330
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples( 4 );
    format.setDepthBufferSize( 24 );
    CanonicalGLWindowImpl *modelWindow = 0;
    modelWindow = new CanonicalGLWindowImpl();
    modelWindow->setFormat(format);
    modelWindow->resize(2048,1024);
    modelWindow->show();
    return a.exec();
}
