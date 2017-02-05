#ifndef CANONICALMONOSCOPICRENDERER_H
#define CANONICALMONOSCOPICRENDERER_H

#include "scene.h"


class CanonicalMonoscopicRenderer
{
public:
    CanonicalMonoscopicRenderer();
    virtual void draw(Scene* s);
    virtual void initialize();
    void setProjectionMatrix(QMatrix4x4 p);
    void setViewMatrix(QMatrix4x4 v);
    void setCameraPosition(QVector3D p);
    void setCameraOrientation(QQuaternion q);

    QOpenGLShaderProgram shaderProgram;


protected:
    //gl attributes
    QMatrix4x4 view;
    QMatrix4x4 projection;
    QVector3D cameraPosition;
    QQuaternion cameraOrientation;

};

#endif // CANONICALMONOSCOPICRENDERER_H
