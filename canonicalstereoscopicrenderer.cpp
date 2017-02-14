#include "canonicalstereoscopicrenderer.h"

#define MAX_EYE_SEPARATION 7.0f
CanonicalStereoscopicRenderer::CanonicalStereoscopicRenderer()
{
    normalizedEyeSeparation = 1.0f;
}

void CanonicalStereoscopicRenderer::setNormalizedEyeSeparation(float e) {
    e = e > 1.0f ? 1.0f : (e < 0.0f ? 0.0f : e);
    normalizedEyeSeparation = e;
}
float CanonicalStereoscopicRenderer::getNormalizedEyeSeparation() {
    return normalizedEyeSeparation;
}


void CanonicalStereoscopicRenderer::draw(Scene* s) {
    GLint viewport[4] = {
        0,0,0,0
    };



    //bind left image depth  buffer for reading
    //GL.glActiveTexture(GL_TEXTURE2);
    //GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[DepthRight]);
    //shaderProgram.setUniformValue("leftImageDepthSampler" , 2);

    //set projection matrix (which is the same for both eyes)
    shaderProgram.setUniformValue( "P", projection );
    shaderProgram.setUniformValue( "height", 256.0f );
    shaderProgram.setUniformValue( "width", 512.0f );
    GL.glGetIntegerv(GL_VIEWPORT,viewport);
    //position the viewports on the screen somehow
    int w = viewport[2];
    int h = viewport[3];
    //qDebug() << w << " " << h;
    float eyeSeparation = MAX_EYE_SEPARATION * normalizedEyeSeparation;
    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);

    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);



    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[0]); //left
    //draw to attachments
    GLenum drawBufs[2] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1};
    GL.glDrawBuffers(2,drawBufs);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);



    //draw left eye
    shaderProgram.setUniformValue("eyeIndex",0);
    shaderProgram.setUniformValue("eyeSeparation",eyeSeparation);
    GL.glViewport( 0, 0,512,256 );
    view.translate( cameraOrientation.rotatedVector( QVector3D(eyeSeparation / 2.0f,0.0f,0.0f) ));
    setCameraPosition(cameraPosition - QVector3D(eyeSeparation / 2.0f,0.0f,0.0f));
    shaderProgram.setUniformValue( "V", view );

    //perform z prepass
    //GL.glDrawBuffer(GL_COLOR_ATTACHMENT2); //draw into "reprojection x" buffer
    shaderProgram.setUniformValue("zPrepass",true);
    s->draw(&shaderProgram,view,projection, OPAQUE);



    //first draw opaque, then transparent

    GL.glDrawBuffer(GL_COLOR_ATTACHMENT0); //into left color buffer only to avoid flushing the 2nd color buffer

    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glDisable(GL_BLEND);
    s->draw(&shaderProgram,view,projection, OPAQUE);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,view,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);





    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    //draw to attachments
    GL.glDrawBuffers(2,drawBufs);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);



    //bind "reprojection x" buffer for reading from the LEFT fbo
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][ReprojectedX]);
    shaderProgram.setUniformValue("reprojectionCoordinateSampler" , 0);
    //bind left image color buffer for reading
    GL.glActiveTexture(GL_TEXTURE1);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Color]);
    shaderProgram.setUniformValue("leftImageSampler" , 1);

    //draw right eye
    //GL.glDrawBuffer(GL_COLOR_ATTACHMENT1); //draw into right color buffer
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,512,256 );
    view.translate( cameraOrientation.rotatedVector( QVector3D(-eyeSeparation,0.0f,0.0f) ));
    setCameraPosition(cameraPosition - QVector3D(-eyeSeparation,0.0f,0.0f));
    shaderProgram.setUniformValue( "V", view );
    shaderProgram.setUniformValue("zPrepass",true);
    GL.glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    s->draw(&shaderProgram,view,projection, OPAQUE );

    //blit framebuffer data to screen
    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[1]);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GLenum status = GL.glGetError();
    //qDebug() << status;
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//right camera
    GL.glBlitFramebuffer(0,0,512,256,
                         0,0,512,256, GL_COLOR_BUFFER_BIT,GL_NEAREST);

    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[0]);
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//left camera
    GL.glBlitFramebuffer(0,0  ,512,256,
                         0,256,512,512,GL_COLOR_BUFFER_BIT,GL_NEAREST);
}

void CanonicalStereoscopicRenderer::initialize() {
    CanonicalMonoscopicRenderer::initialize();
    initializeFBO(0);
    initializeFBO(1);

    qDebug() << "color: " << GL.glGetFragDataLocation(shaderProgram.programId(), "color");
    qDebug() << "reprojectedX: " << GL.glGetFragDataLocation(shaderProgram.programId(), "reprojectedX");

    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);
}
void CanonicalStereoscopicRenderer::initializeFBO(int fboIndex) {
    GLenum status = GL.glGetError();

    GL.glGenTextures(NumRenderbuffers,renderbuffers[fboIndex]);
    //GL.glGenRenderbuffers(1,&renderbuffers[Stencil]);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Color]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,512,256,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][ReprojectedX]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_RGBA32F  ,512,256,0,    GL_RGBA   , GL_FLOAT,NULL);
    GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_R32F  ,512,256,0,    GL_RED   , GL_FLOAT,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Depth]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_DEPTH_COMPONENT24  ,512,256,0,  GL_DEPTH_COMPONENT  ,GL_FLOAT,NULL);


    //GL.glBindRenderbuffer(GL_RENDERBUFFER,renderbuffers[Stencil]);
    //GL.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL,512,256);

    GL.glGenFramebuffers(1,&fbos[fboIndex]);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[fboIndex]);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderbuffers[fboIndex][Color],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,renderbuffers[fboIndex][ReprojectedX],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,renderbuffers[fboIndex][Depth],0);
    //GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,renderbuffers[Stencil]);

    status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    qDebug() << status;



}
