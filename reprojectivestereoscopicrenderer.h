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
    GLuint getReprojectedImage();
protected:
    enum {Color, Exchange, Depth ,NumRenderbuffers};
    enum {Left, Right,NumFBOs};
    void initializeFBO(int fboIndex, int w, int h);
    GLuint fbos[NumFBOs];
    GLuint renderbuffers[NumFBOs][NumRenderbuffers];

    float normalizedEyeSeparation;

   // QElapsedTimer timer;

};


#endif // REPROJECTIVESTEREOSCOPICRENDERER_H
