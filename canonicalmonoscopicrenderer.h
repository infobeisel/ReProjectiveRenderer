#ifndef CANONICALMONOSCOPICRENDERER_H
#define CANONICALMONOSCOPICRENDERER_H

#include "scene.h"
#include "renderer.h"

class CanonicalMonoscopicRenderer : public Renderer
{
public:
    CanonicalMonoscopicRenderer();
    void draw(Scene* s) override;
    void initialize() override;
    void initialize(int w, int h) override;
    void setNormalizedEyeSeparation(float e) override;
    float getNormalizedEyeSeparation() override;
    void setProjectionMatrix(QMatrix4x4 p) override;
    void setViewMatrix(QMatrix4x4 v) override;
    void setCameraPosition(QVector3D p) override;
    void setCameraOrientation(QQuaternion q) override;
    QString configTags() override;


protected:

    //gl attributes
    QMatrix4x4 view;
    QMatrix4x4 projection;
    QVector3D cameraPosition;
    QQuaternion cameraOrientation;

    QOpenGLShaderProgram zPrepassShaderProgram;

};

#endif // CANONICALMONOSCOPICRENDERER_H
