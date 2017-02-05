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
    //right eye
    GLuint rightFBO;
    GLuint rightRenderbuffer;

};

#endif // CANONICALSTEROSCOPICRENDERER_H
