#include "canonicalstereoscopicrenderer.h"
#include "configuration.h"
#include <QtCore/qmath.h>
CanonicalStereoscopicRenderer::CanonicalStereoscopicRenderer()
{
    normalizedEyeSeparation = 1.0f;
}
void CanonicalStereoscopicRenderer::toggleLeftZPrepass() {
     leftEyeZPrepass = !leftEyeZPrepass;
}

void CanonicalStereoscopicRenderer::setProjectionMatrix(float fov,float aspect, float near, float far) {
    projection.setToIdentity();
    projection.perspective(
                fov,          // field of vision
                aspect,         // aspect ratio
                near,           // near clipping plane
                far);       // far clipping plane

    //derive left and right eye projection matrices
    leftProjection = QMatrix4x4();
    rightProjection = QMatrix4x4();
    projection.copyDataTo(leftProjection.data());
    projection.copyDataTo(rightProjection.data());
    qreal radians = (fov / 2.0f) * M_PI / 180.0f;
    float eyeSeparation = Configuration::instance().MaxEyeSeparation * normalizedEyeSeparation;
    float nearWidth = qSin(radians / 2.0f) * 2.0f * near;

    /*
     * set 2n/r-l and r+l/r-l
     *
     * */
    QVector4D firstRowLeft = leftProjection.row(0);
    QVector4D firstRowRight = rightProjection.row(0);
    //firstRowLeft.setX( 2.0f * near / ( r - l + eyeSeparation));
    //firstRowRight.setX(2.0f * near / ( r - l + eyeSeparation));
    firstRowLeft.setZ( nearWidth * (( eyeSeparation) / ( 2.0f * near )));
    firstRowRight.setZ( nearWidth * (( -eyeSeparation) / ( 2.0f * near )));
    leftProjection.setRow(0,firstRowLeft);
    rightProjection.setRow(0,firstRowRight);

}


void CanonicalStereoscopicRenderer::toggleRightZPrepass() {
    rightEyeZPrepass = !rightEyeZPrepass;
}
void CanonicalStereoscopicRenderer::setNormalizedEyeSeparation(float e) {
    e = e > 1.0f ? 1.0f : (e < 0.0f ? 0.01f : e);
    normalizedEyeSeparation = e;
}
float CanonicalStereoscopicRenderer::getNormalizedEyeSeparation() {
    return normalizedEyeSeparation;
}
QString CanonicalStereoscopicRenderer::configTags() {
    std::stringstream ss;
    ss << "CanonicalStereoscopicRenderer Renderer, Left Z Prepass " << (leftEyeZPrepass ? "true" : "false")  << ", Right Z Prepass " << (rightEyeZPrepass ? "true" : "false") ;
    return QString::fromStdString(ss.str());
}

void CanonicalStereoscopicRenderer::draw(Scene* s) {
    GLint viewport[4] = {
        0,0,0,0
    };
    GL.glGetIntegerv(GL_VIEWPORT,viewport);

    //prepare view matrices for left and right eye
    float eyeSeparation = Configuration::instance().MaxEyeSeparation * normalizedEyeSeparation;


    QVector3D up = cameraOrientation.rotatedVector(QVector3D(0.0f,1.0f,0.0f));
    QVector3D forward = cameraOrientation.rotatedVector(QVector3D(0.0f,0.0f,-1.0f));
    QVector3D right = cameraOrientation.rotatedVector(QVector3D(1.0f,0.0f,0.0f));

    QMatrix4x4 viewLeft;
    viewLeft.setToIdentity();
    viewLeft.lookAt(cameraPosition - (right * eyeSeparation / 2.0f) , cameraPosition - (right * eyeSeparation / 2.0f) + forward,up);
    QMatrix4x4 viewRight;
    viewRight.setToIdentity();
    viewRight.lookAt(cameraPosition + (right * eyeSeparation / 2.0f) , cameraPosition + (right * eyeSeparation / 2.0f) + forward,up);
    QVector3D originalCameraPosition = cameraPosition;
    QVector3D leftCameraPosition = originalCameraPosition - (right * eyeSeparation / 2.0f);
    QVector3D rightCameraPosition = originalCameraPosition + (right * eyeSeparation / 2.0f);


    //position the viewports on the screen somehow
    int w = viewport[2];
    int h = viewport[3];


    //set projection matrix (which is the same for both eyes)
    shaderProgram.setUniformValue( "P", projection );
    shaderProgram.setUniformValue( "height", (float)h  );
    shaderProgram.setUniformValue( "width", (float)w );
    GL.glGetIntegerv(GL_VIEWPORT,viewport);

    //qDebug() << w << " " << h;

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
    GL.glViewport( 0, 0,w,h );

    setCameraPosition(leftCameraPosition);
    shaderProgram.setUniformValue( "V", viewLeft );

    //first draw opaque, then transparent. store depth values in exchange buffer
    GL.glDisable(GL_BLEND);

    //zprepass
    if(rightEyeZPrepass) {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LEQUAL);

        GL.glDrawBuffer(GL_NONE);
        zPrepassShaderProgram.bind();
        s->bind(&zPrepassShaderProgram);
        s->draw(&zPrepassShaderProgram,viewLeft,leftProjection, OPAQUE);

        shaderProgram.bind();
        s->bind(&shaderProgram);
        GL.glDrawBuffers(2,drawBufs);
    } else {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LESS);
    }
    s->draw(&shaderProgram,viewLeft,leftProjection, OPAQUE);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewLeft,leftProjection, TRANSPARENT);
    GL.glDisable(GL_BLEND);

    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    //draw to attachments
    GL.glDrawBuffers(2,drawBufs);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //draw right eye
    //GL.glDrawBuffer(GL_COLOR_ATTACHMENT1); //draw into right color buffer
    shaderProgram.setUniformValue("eyeIndex",0);
    GL.glViewport( 0, 0,w,h );

    setCameraPosition(rightCameraPosition);
    shaderProgram.setUniformValue( "V", viewRight );
    GL.glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    //zprepass
    if(rightEyeZPrepass) {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LEQUAL);

        GL.glDrawBuffer(GL_NONE);
        zPrepassShaderProgram.bind();
        s->bind(&zPrepassShaderProgram);
        s->draw(&zPrepassShaderProgram,viewRight,rightProjection, OPAQUE);

        shaderProgram.bind();
        s->bind(&shaderProgram);
        GL.glDrawBuffers(2,drawBufs);
    } else {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LESS);
    }

    s->draw(&shaderProgram,viewRight,rightProjection, OPAQUE );
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewRight,rightProjection, TRANSPARENT);
    GL.glDisable(GL_BLEND);



    //blit framebuffer data to screen
    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[1]);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GLenum status = GL.glGetError();
    //qDebug() << status;
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//right camera
    GL.glBlitFramebuffer(0,0,w,h/2,
                         0,0,w,h/2, GL_COLOR_BUFFER_BIT,GL_NEAREST);


    //blit framebuffer data to screen
    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[0]);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GLenum status = GL.glGetError();
    //qDebug() << status;
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//left camera
    GL.glBlitFramebuffer(0,0,w,h/2,
                         0,h/2,w,h, GL_COLOR_BUFFER_BIT,GL_NEAREST);


}

void CanonicalStereoscopicRenderer::initialize(int w, int h) {

    leftEyeZPrepass = false;
    rightEyeZPrepass = false;
    CanonicalMonoscopicRenderer::initialize();
    GL.glViewport( 0, 0,w,h );
    qDebug() << w << " " << h;
    //if the framebuffers got initialized already, deallocate the memory
    GL.glDeleteRenderbuffers(NumRenderbuffers,renderbuffers[0]);
    GL.glDeleteRenderbuffers(NumRenderbuffers,renderbuffers[1]);
    GL.glDeleteFramebuffers(NumFBOs,fbos);

    initializeFBO(0,w,h);
    initializeFBO(1,w,h);

    qDebug() << "color: " << GL.glGetFragDataLocation(shaderProgram.programId(), "color");
    qDebug() << "exchangeBuffer: " << GL.glGetFragDataLocation(shaderProgram.programId(), "exchangeBuffer");

    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);
}
void CanonicalStereoscopicRenderer::initializeFBO(int fboIndex, int w , int h) {
    GLenum status = GL.glGetError();

    GL.glGenTextures(NumRenderbuffers,renderbuffers[fboIndex]);
    //GL.glGenRenderbuffers(1,&renderbuffers[Stencil]);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Color]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Exchange]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_RGBA32F  ,w,h,0,    GL_RGBA   , GL_FLOAT,NULL);
    //GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_R32F  ,w,h,0,    GL_RED   , GL_FLOAT,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Depth]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,  GL_DEPTH_COMPONENT24  ,w,h,0,  GL_DEPTH_COMPONENT  ,GL_FLOAT,NULL);


    //GL.glBindRenderbuffer(GL_RENDERBUFFER,renderbuffers[Stencil]);
    //GL.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL,w,h);

    GL.glGenFramebuffers(1,&fbos[fboIndex]);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[fboIndex]);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderbuffers[fboIndex][Color],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,renderbuffers[fboIndex][Exchange],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,renderbuffers[fboIndex][Depth],0);
    //GL.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,renderbuffers[Stencil]);

    status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    qDebug() << status;



}
