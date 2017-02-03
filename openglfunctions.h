#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#define GL OpenGLFunctions::instance()

#include <QOpenGLFunctions_4_1_Core>
#include <QDebug>

class OpenGLFunctions : public QOpenGLFunctions_4_1_Core
{

public:
    static OpenGLFunctions& instance() {
        static OpenGLFunctions instance;
        return instance;
    }
    //delete assign operator and copy constructor
    OpenGLFunctions(OpenGLFunctions const&) = delete;
    void operator=(OpenGLFunctions const&)  = delete;

private:
    OpenGLFunctions()
    {
        qDebug() << "initialize GL Functions";
        initializeOpenGLFunctions();
    }
};

#endif // OPENGLFUNCTIONS_H
