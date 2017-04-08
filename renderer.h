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
    virtual void initialize(int width, int height);
    virtual void toggleDebugMode();
    virtual void setNormalizedEyeSeparation(float e);
    virtual float getNormalizedEyeSeparation();
    virtual void setProjectionMatrix(float fov,float aspect, float near, float far);
    virtual void setViewMatrix(QMatrix4x4 v);
    virtual void setCameraPosition(QVector3D p);
    virtual void setCameraOrientation(QQuaternion q);
    virtual QOpenGLShaderProgram* getShaderProgram();
    virtual QString configTags();//returns config tags which were set during runtime (e.g. debug mode)
    Renderer();
    ~Renderer();
protected:

    //gl attributes
    QOpenGLShaderProgram shaderProgram;

};

#endif // RENDERER_H
