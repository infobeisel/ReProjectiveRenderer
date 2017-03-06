#include "canonicalglwindowimpl.h"
#include <QGuiApplication>
#include <QSurfaceFormat>
#include "configuration.h"
int main(int argc, char *argv[])
{
    Configuration::instance().load("config");


    QSurfaceFormat format;
    format.setVersion(4,1); //core version 410
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples( 4 );
    format.setDepthBufferSize( 24 );
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);
    QGuiApplication a(argc, argv);
    CanonicalGLWindowImpl *modelWindow = 0;
    modelWindow = new CanonicalGLWindowImpl();
    modelWindow->setFormat(format);
    modelWindow->resize(Configuration::instance().WindowWidth,Configuration::instance().WindowHeight);
    modelWindow->show();



    return a.exec();
}
