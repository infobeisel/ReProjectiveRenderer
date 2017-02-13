#ifndef CANONICALSTEROSCOPICRENDERER_H
#define CANONICALSTEROSCOPICRENDERER_H

#include "canonicalmonoscopicrenderer.h"
#include <QElapsedTimer>


class CanonicalStereoscopicRenderer : public CanonicalMonoscopicRenderer
{
public:
    CanonicalStereoscopicRenderer();

    void draw(Scene* s) override;
    void initialize() override;

    void setNormalizedEyeSeparation(float e) override;
    float getNormalizedEyeSeparation() override;
private:
    enum {ColorLeft, ColorRight,ReprojectedX,  DepthLeft, DepthRight, Stencil, NumRenderbuffers};


    GLuint fbo;
    GLuint renderbuffers[NumRenderbuffers];

    float normalizedEyeSeparation;

   // QElapsedTimer timer;

};

#endif // CANONICALSTEROSCOPICRENDERER_H
