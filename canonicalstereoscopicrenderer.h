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
    enum {Color, ReprojectedX, Depth ,NumRenderbuffers};
    enum {Left, Right,NumFBOs};
    void initializeFBO(int fboIndex);
    GLuint fbos[NumFBOs];
    GLuint renderbuffers[NumFBOs][NumRenderbuffers];

    float normalizedEyeSeparation;

   // QElapsedTimer timer;

};

#endif // CANONICALSTEROSCOPICRENDERER_H
