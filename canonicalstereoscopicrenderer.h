#ifndef CANONICALSTEROSCOPICRENDERER_H
#define CANONICALSTEROSCOPICRENDERER_H

#include "canonicalmonoscopicrenderer.h"



class CanonicalStereoscopicRenderer : public CanonicalMonoscopicRenderer
{
public:
    CanonicalStereoscopicRenderer();

    void draw(Scene* s) override;
    void initialize() override;
private:
    enum {ColorLeft, ColorRight,ReprojectedX,  Depth, Stencil, NumRenderbuffers};


    GLuint fbo;
    GLuint renderbuffers[NumRenderbuffers];


};

#endif // CANONICALSTEROSCOPICRENDERER_H
