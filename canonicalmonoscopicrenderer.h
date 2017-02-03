#ifndef CANONICALMONOSCOPICRENDERER_H
#define CANONICALMONOSCOPICRENDERER_H

#include "scene.h"


class CanonicalMonoscopicRenderer
{
public:
    CanonicalMonoscopicRenderer();
    void draw(Scene* s);
    void initialize();
    void setProjectionMatrix(QMatrix4x4 p);
    void setViewMatrix(QMatrix4x4 v);
    void setCameraPosition(QVector3D p);

    QOpenGLShaderProgram shaderProgram;


protected:
    //gl attributes
    QMatrix4x4 view;
    QMatrix4x4 projection;
    QVector3D cameraPosition;

};

#endif // CANONICALMONOSCOPICRENDERER_H
