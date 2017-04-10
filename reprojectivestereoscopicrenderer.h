#ifndef REPROJECTIVESTEREOSCOPICRENDERER_H
#define REPROJECTIVESTEREOSCOPICRENDERER_H


#include "canonicalmonoscopicrenderer.h"
#include "canonicalstereoscopicrenderer.h"
#include <QElapsedTimer>
#include "utils/pixelcounter.h"

class ReprojectiveStereoscopicRenderer : public CanonicalStereoscopicRenderer
{
public:
    ReprojectiveStereoscopicRenderer();

    void draw(Scene* s) override;
    void initialize(int w, int h) override;

    void setNormalizedEyeSeparation(float e) override;
    float getNormalizedEyeSeparation() override;
    QString configTags() override;


    GLuint getRightImage();
    GLuint getLeftImage();
protected:
    enum {Color, Exchange, Depth ,NumRenderbuffers};
    enum {Left, Right,NumFBOs};
    void initializeFBO(int fboIndex, int w, int h);
    GLuint fbos[NumFBOs];
    GLuint renderbuffers[NumFBOs][NumRenderbuffers];

    float normalizedEyeSeparation;



    //precompiled shader objects
    QOpenGLShader* fullRenderOnly;
    QOpenGLShader* fullRenderWithExchangeBufferWrites;
    QOpenGLShader* reprojectionOnly;

    QOpenGLShaderProgram copyColorBufToStencilBuf;
    Scene* plane;



};


#endif // REPROJECTIVESTEREOSCOPICRENDERER_H

