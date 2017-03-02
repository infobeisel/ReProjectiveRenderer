#ifndef REPROJECTIVESTEREOSCOPICRENDERER_H
#define REPROJECTIVESTEREOSCOPICRENDERER_H


#include "canonicalmonoscopicrenderer.h"
#include <QElapsedTimer>
#include "utils/pixelcounter.h"

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

    float normalizedEyeSeparation;

    bool leftEyeZPrepass;
    bool rightEyeZPrepass;
    bool debugMode;

   // QElapsedTimer timer;

};


#endif // REPROJECTIVESTEREOSCOPICRENDERER_H

