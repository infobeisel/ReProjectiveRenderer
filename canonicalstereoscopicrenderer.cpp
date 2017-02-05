#include "canonicalstereoscopicrenderer.h"

CanonicalStereoscopicRenderer::CanonicalStereoscopicRenderer()
{

}


void CanonicalStereoscopicRenderer::draw(Scene* s) {

    GLint viewport[4] = {
        0,0,0,0
    };

    GL.glGetIntegerv(GL_VIEWPORT,viewport);
    qDebug() << view;

    //commented lines create an extra frame buffer object to which the two images are rendered

    //GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,rightFBO);
    GL.glClearColor(1.0,0.0,0.0,1.0);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //position the viewports on the screen somehow
    int w = viewport[2] / 4;
    int h = viewport[2] / 4;
    int posy = 0;
    int posx1 = 0;
    int posx2 = w + 10;


    //float eyeSeparation = 0.07f;
    float eyeSeparation = 7.0f; // 7 cm ?

    //draw left eye first
    GL.glViewport(posx1,posy,w,h);
    view.translate( cameraOrientation.rotatedVector( QVector3D(- eyeSeparation / 2.0f,0.0f,0.0f) ));

    //first draw opaque, then transparent
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glDisable(GL_BLEND);
    s->draw(&shaderProgram,view,projection, OPAQUE);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,view,projection, TRANSPARENT);

    //draw right eye
    GL.glViewport(posx2,posy,w,h);
    view.translate( cameraOrientation.rotatedVector( QVector3D(eyeSeparation,0.0f,0.0f) ));
    //first draw opaque, then transparent
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glDisable(GL_BLEND);
    s->draw(&shaderProgram,view,projection, OPAQUE);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,view,projection, TRANSPARENT);

    //read from renderbuffer and draw to window-system framebuffer
    //GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,rightFBO);
    //GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GL.glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    //GL.glClearColor(0.0,0.0,1.0,1.0);

    //(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
    //GL.glBlitFramebuffer(0,0,255,255,0,0,255,255,GL_COLOR_BUFFER_BIT,GL_NEAREST);


}

void CanonicalStereoscopicRenderer::initialize() {
    CanonicalMonoscopicRenderer::initialize();

  //  rightFBO;
    //     rightRenderbuffer;
    GL.glGenRenderbuffers(1,&rightRenderbuffer);
    GL.glBindRenderbuffer(GL_RENDERBUFFER,rightRenderbuffer);
    GL.glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA,256,256);

    GL.glGenFramebuffers(1,&rightFBO);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,rightFBO);
    GL.glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,rightRenderbuffer);


}
