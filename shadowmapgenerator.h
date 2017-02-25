#ifndef SHADOWMAPGENERATOR_H
#define SHADOWMAPGENERATOR_H
#include "canonicalmonoscopicrenderer.h"

class ShadowMapGenerator : public CanonicalMonoscopicRenderer
{
public:
    ShadowMapGenerator();
    void initialize() override;
    void initialize(int w, int h) override;
    /**
     * @brief generate a shadow map from the given scene, use the first occuring directional light source
     * @param s
     */
    void draw(Scene* s) override;
    void saveToImage(QString path);
    void setShadowMapVariables(QOpenGLShaderProgram* toShader);


private:
    enum {Color,Depth ,NumRenderbuffers};
    enum {Shadow,NumFBOs};
    void initializeFBO(int fboIndex, int w, int h);
    GLuint fbos[NumFBOs];
    GLuint renderbuffers[NumFBOs][NumRenderbuffers];
    int mapResolution[2];
};

#endif // SHADOWMAPGENERATOR_H
