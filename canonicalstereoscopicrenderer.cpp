#include "canonicalstereoscopicrenderer.h"

CanonicalStereoscopicRenderer::CanonicalStereoscopicRenderer()
{

}


void CanonicalStereoscopicRenderer::draw(Scene* s) {
    GLint viewport[4] = {
        0,0,0,0
    };
    //set projection matrix (which is the same for both eyes)
    shaderProgram.setUniformValue( "P", projection );
    shaderProgram.setUniformValue( "height", 256.0f );
    shaderProgram.setUniformValue( "width", 512.0f );
    GL.glGetIntegerv(GL_VIEWPORT,viewport);
    //position the viewports on the screen somehow
    int w = viewport[2];
    int h = viewport[3];
    //qDebug() << w << " " << h;
    //float eyeSeparation = 0.07f;
    float eyeSeparation = 7.0f; // 7 cm ?
    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);

    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //draw left eye
    shaderProgram.setUniformValue("eyeIndex",0);
    shaderProgram.setUniformValue("eyeSeparation",eyeSeparation);
    GL.glViewport( 0, 0,512,256 );
    view.translate( cameraOrientation.rotatedVector( QVector3D(-eyeSeparation / 2.0f,0.0f,0.0f) ));
    shaderProgram.setUniformValue( "V", view );

    //perform z prepass
    GL.glDrawBuffer(GL_COLOR_ATTACHMENT2); //draw into "reprojection x" buffer
    shaderProgram.setUniformValue("zPrepass",true);
    s->draw(&shaderProgram,view,projection, OPAQUE );

    //first draw opaque, then transparent
    GL.glDrawBuffer(GL_COLOR_ATTACHMENT0); //draw into left color buffer
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glDisable(GL_BLEND);
    s->draw(&shaderProgram,view,projection, OPAQUE);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,view,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);



    //draw right eye
    GL.glDrawBuffer(GL_COLOR_ATTACHMENT1); //draw into right color buffer
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,512,256 );
    view.translate( cameraOrientation.rotatedVector( QVector3D(eyeSeparation,0.0f,0.0f) ));
    shaderProgram.setUniformValue( "V", view );
    shaderProgram.setUniformValue("zPrepass",true);
    GL.glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    s->draw(&shaderProgram,view,projection, OPAQUE );

    //blit framebuffer data to screen
    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GLenum status = GL.glGetError();
    //qDebug() << status;
    GL.glReadBuffer(GL_COLOR_ATTACHMENT1);//right camera
    GL.glBlitFramebuffer(0,0,512,256,
                         0,0,512,256, GL_COLOR_BUFFER_BIT,GL_NEAREST);
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//left camera
    GL.glBlitFramebuffer(0,0  ,512,256,
                         0,256,512,512,GL_COLOR_BUFFER_BIT,GL_NEAREST);

}

void CanonicalStereoscopicRenderer::initialize() {
    CanonicalMonoscopicRenderer::initialize();

    GLenum status = GL.glGetError();

    //left eye: 4 render textures (left color, right color, reprojected x coordinates, depth) ,  1 stencil buffer
    GL.glGenTextures(4,renderbuffers);
    GL.glGenRenderbuffers(1,&renderbuffers[Stencil]);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[ColorLeft]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,512,256,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[ReprojectedX]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_R32F  ,512,256,0,    GL_RED   , GL_FLOAT,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[ColorRight]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,512,256,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[Depth]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_DEPTH_COMPONENT24  ,512,256,0,  GL_DEPTH_COMPONENT  ,GL_FLOAT,NULL);

    GL.glBindRenderbuffer(GL_RENDERBUFFER,renderbuffers[Stencil]);
    GL.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL ,512,256);

    GL.glGenFramebuffers(1,&fbo);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderbuffers[ColorLeft],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,renderbuffers[ColorRight],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,renderbuffers[ReprojectedX],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,renderbuffers[Depth],0);
    GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,renderbuffers[Stencil]);

    //bind "reprojection x" buffer for reading
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[ReprojectedX]);
    shaderProgram.setUniformValue("reprojectionCoordinateSampler" , 0);

    status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    qDebug() << status;

    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);
}
