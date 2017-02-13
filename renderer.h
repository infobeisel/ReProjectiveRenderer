#ifndef RENDERER_H
#define RENDERER_H
class Scene;
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

class Renderer
{
public:

    virtual void draw(Scene* s);
    virtual void initialize();
    virtual void setNormalizedEyeSeparation(float e);
    virtual float getNormalizedEyeSeparation();
    virtual void setProjectionMatrix(QMatrix4x4 p);
    virtual void setViewMatrix(QMatrix4x4 v);
    virtual void setCameraPosition(QVector3D p);
    virtual void setCameraOrientation(QQuaternion q);
    virtual QOpenGLShaderProgram* getShaderProgram();
    Renderer();
    ~Renderer();
protected:

    //gl attributes
    QOpenGLShaderProgram shaderProgram;

};

#endif // RENDERER_H
