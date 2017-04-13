#ifndef REPROJECTIONERRORRENDERER_H
#define REPROJECTIONERRORRENDERER_H
#include "reprojectivestereoscopicrenderer.h"

class ReprojectionErrorRenderer : public ReprojectiveStereoscopicRenderer
{
public:
    ReprojectionErrorRenderer();
    void draw(Scene* s) override;
    QString configTags() override;
    void setProjectionMatrix(float fov,float aspect, float near, float far) override;

};

#endif // REPROJECTIONERRORRENDERER_H
