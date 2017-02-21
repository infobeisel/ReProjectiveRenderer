#include "reprojectionerrorrenderer.h"
#define MAX_EYE_SEPARATION 7.0f

ReprojectionErrorRenderer::ReprojectionErrorRenderer()
{
    normalizedEyeSeparation = 1.0f;
}
void ReprojectionErrorRenderer::draw(Scene* s) {
    GLint viewport[4] = {
        0,0,0,0
    };
    GL.glGetIntegerv(GL_VIEWPORT,viewport);
    //prepare view matrices for left and right eye
    float eyeSeparation = MAX_EYE_SEPARATION * normalizedEyeSeparation;
    QVector3D up = cameraOrientation.rotatedVector(QVector3D(0.0f,1.0f,0.0f));
    QVector3D forward = cameraOrientation.rotatedVector(QVector3D(0.0f,0.0f,-1.0f));
    QVector3D right = cameraOrientation.rotatedVector(QVector3D(1.0f,0.0f,0.0f));
    QMatrix4x4 viewLeft;
    viewLeft.setToIdentity();
    viewLeft.lookAt(cameraPosition - (right * eyeSeparation / 2.0f) , cameraPosition - (right * eyeSeparation / 2.0f) + forward,up);
    QMatrix4x4 viewRight;
    viewRight.setToIdentity();
    viewRight.lookAt(cameraPosition + (right * eyeSeparation / 2.0f) , cameraPosition + (right * eyeSeparation / 2.0f) + forward,up);
    //position the viewports on the screen somehow
    int w = viewport[2];
    int h = viewport[3];
    //depth difference threshold to recognize non-reusable fragments
    //shaderProgram.setUniformValue( "depthDifThreshold", eyeSeparation );
    shaderProgram.setUniformValue( "depthThreshold", eyeSeparation /  10000.0f / 1000.0f );
    //set projection matrix (which is the same for both eyes)
    shaderProgram.setUniformValue( "P", projection );
    shaderProgram.setUniformValue( "height", (float)h / 2.0f );
    shaderProgram.setUniformValue( "width", (float)w );
    GL.glGetIntegerv(GL_VIEWPORT,viewport);
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
    GL.glViewport( 0, 0,w,h/2 );
    setCameraPosition(cameraPosition - (right * eyeSeparation / 2.0f) );
    shaderProgram.setUniformValue( "V", viewLeft );
    //first draw opaque. store depth values in exchange buffer
    shaderProgram.setUniformValue("zPrepass",true);
    GL.glDisable(GL_BLEND);
    s->draw(&shaderProgram,viewLeft,projection, OPAQUE);


    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    //draw to attachments
    GL.glDrawBuffers(2,drawBufs);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //bind "exchange" buffer for reading from the LEFT fbo
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Exchange]);
    shaderProgram.setUniformValue("exchangeBufferSampler" , 0);
    //bind left image color buffer for reading
    GL.glActiveTexture(GL_TEXTURE1);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Color]);
    shaderProgram.setUniformValue("leftImageSampler" , 1);

    //draw right eye
    //GL.glDrawBuffer(GL_COLOR_ATTACHMENT1); //draw into right color buffer
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,w,h/2 );
    setCameraPosition(cameraPosition + (right * eyeSeparation / 2.0f));
    shaderProgram.setUniformValue( "V", viewRight );
    shaderProgram.setUniformValue("zPrepass",true);
    GL.glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    s->draw(&shaderProgram,viewRight,projection, OPAQUE );


    //transparent objects
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[0]); //left
    GL.glDrawBuffers(2,drawBufs);
    shaderProgram.setUniformValue("eyeIndex",0);
    GL.glViewport( 0, 0,w,h/2 );
    setCameraPosition(cameraPosition - (right * eyeSeparation / 2.0f) );
    shaderProgram.setUniformValue( "V", viewLeft );
    GL.glEnable(GL_BLEND);
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewLeft,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);

    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    GL.glDrawBuffers(2,drawBufs);
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,w,h/2 );
    setCameraPosition(cameraPosition + (right * eyeSeparation / 2.0f));
    shaderProgram.setUniformValue( "V", viewRight );
    //in order to determine the reprojection error, transparent objects are not included because they are always rendered anyway
    /*GL.glEnable(GL_BLEND);
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewRight,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);*/





    //draw right eye, this time without reprojection and with blending over the reprojected image to determine the reprojection error
    //GL.glDrawBuffer(GL_COLOR_ATTACHMENT1); //draw into right color buffer
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,w,h/2 );

    setCameraPosition(cameraPosition + (right * eyeSeparation / 2.0f));
    shaderProgram.setUniformValue( "V", viewRight );
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc(GL_ONE, GL_ONE);
    s->draw(&shaderProgram,viewRight,projection, OPAQUE );
     GL.glDisable(GL_BLEND);
    /*GL.glEnable(GL_BLEND);
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewRight,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);*/



    //blit framebuffer data to screen
    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[1]);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GLenum status = GL.glGetError();
    //qDebug() << status;
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//right camera
    GL.glBlitFramebuffer(0,0,w,h/2,
                         0,0,w,h/2, GL_COLOR_BUFFER_BIT,GL_NEAREST);

    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[0]);
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//left camera
    GL.glBlitFramebuffer(0,0  ,w,h/2,
                         0,h/2,w,h,GL_COLOR_BUFFER_BIT,GL_NEAREST);
}

