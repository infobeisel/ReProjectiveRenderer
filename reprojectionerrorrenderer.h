#ifndef REPROJECTIONERRORRENDERER_H
#define REPROJECTIONERRORRENDERER_H
#include "reprojectivestereoscopicrenderer.h"

class ReprojectionErrorRenderer : public ReprojectiveStereoscopicRenderer
{
public:
    ReprojectionErrorRenderer();
    void draw(Scene* s) override;

};

#endif // REPROJECTIONERRORRENDERER_H
