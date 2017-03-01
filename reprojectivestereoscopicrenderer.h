#ifndef REPROJECTIVESTEREOSCOPICRENDERER_H
#define REPROJECTIVESTEREOSCOPICRENDERER_H


#include "canonicalmonoscopicrenderer.h"
#include <QElapsedTimer>
#include "utils/pixelcounter.h"
#include <QOpenGLShader>
class ReprojectiveStereoscopicRenderer : public CanonicalMonoscopicRenderer
{
public:
    ReprojectiveStereoscopicRenderer();

    void draw(Scene* s) override;
    void initialize(int w, int h) override;

    void setNormalizedEyeSeparation(float e) override;
    float getNormalizedEyeSeparation() override;
    void toggleDebugMode() override;
    QString configTags() override;

    void toggleLeftZPrepass();
    void toggleRightZPrepass();

    GLuint getRightImage();
    GLuint getLeftImage();
protected:
    enum {Color, Exchange, Depth ,NumRenderbuffers};
    enum {Left, Right,NumFBOs};
    void initializeFBO(int fboIndex, int w, int h);
    GLuint fbos[NumFBOs];
    GLuint renderbuffers[NumFBOs][NumRenderbuffers];

    //shader which does normal light calculation
    QOpenGLShaderProgram fullRenderShaderOnly;
    //additionally writes necessary values to exchange buffer for later reprojection
    QOpenGLShaderProgram fullRenderShaderWithExchangeBuffer;
    //performs reprojection and reads from provided color and depth buffer
    QOpenGLShaderProgram reprojectionShader;


    float normalizedEyeSeparation;

    bool leftEyeZPrepass;
    bool rightEyeZPrepass;
    bool debugMode;

   // QElapsedTimer timer;

};


#endif // REPROJECTIVESTEREOSCOPICRENDERER_H

