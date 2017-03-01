#ifndef CANONICALSTEROSCOPICRENDERER_H
#define CANONICALSTEROSCOPICRENDERER_H

#include "canonicalmonoscopicrenderer.h"
#include <QElapsedTimer>


class CanonicalStereoscopicRenderer : public CanonicalMonoscopicRenderer
{
public:
    CanonicalStereoscopicRenderer();

    void draw(Scene* s) override;
    void initialize(int w, int h) override;

    void setNormalizedEyeSeparation(float e) override;
    float getNormalizedEyeSeparation() override;
    QString configTags() override;

    void toggleLeftZPrepass();
    void toggleRightZPrepass();

private:
    enum {Color, Exchange, Depth ,NumRenderbuffers};
    enum {Left, Right,NumFBOs};
    void initializeFBO(int fboIndex, int w, int h);
    GLuint fbos[NumFBOs];
    GLuint renderbuffers[NumFBOs][NumRenderbuffers];

    float normalizedEyeSeparation;

   // QElapsedTimer timer;
    bool leftEyeZPrepass;
    bool rightEyeZPrepass;
};

#endif // CANONICALSTEROSCOPICRENDERER_H
