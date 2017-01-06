#include <QGuiApplication>
#include <QSurfaceFormat>
#include "canonicalglwindowimpl.h"
int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    QSurfaceFormat format;
    format.setVersion(3,3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples( 4 );
    format.setDepthBufferSize( 24 );

    CanonicalGLWindowImpl *modelWindow = 0;
    modelWindow = new CanonicalGLWindowImpl();
    modelWindow->setFormat(format);
    modelWindow->resize(1024,1024);
    modelWindow->show();

    return a.exec();
}
