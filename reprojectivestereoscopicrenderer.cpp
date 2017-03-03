#include "reprojectivestereoscopicrenderer.h"
#include <QFile>
ReprojectiveStereoscopicRenderer::ReprojectiveStereoscopicRenderer()
{
    normalizedEyeSeparation = 1.0f;
}

void ReprojectiveStereoscopicRenderer::setNormalizedEyeSeparation(float e) {
    e = e > 1.0f ? 1.0f : (e < 0.0f ? 0.01f : e);
    normalizedEyeSeparation = e;
}
float ReprojectiveStereoscopicRenderer::getNormalizedEyeSeparation() {
    return normalizedEyeSeparation;
}
GLuint ReprojectiveStereoscopicRenderer::getRightImage() {
    return renderbuffers[Right][Color];
}
GLuint ReprojectiveStereoscopicRenderer::getLeftImage() {
    return renderbuffers[Left][Color];
}
QString ReprojectiveStereoscopicRenderer::configTags() {
    std::stringstream ss;
    ss << "Reprojective Renderer, Debug Mode " << (debugMode ? "true" : "false") << ", Left Z Prepass " << (leftEyeZPrepass ? "true" : "false")  << ", Right Z Prepass " << (rightEyeZPrepass ? "true" : "false") ;
    return QString::fromStdString(ss.str());
}

void ReprojectiveStereoscopicRenderer::toggleLeftZPrepass() {
     leftEyeZPrepass = !leftEyeZPrepass;
}

void ReprojectiveStereoscopicRenderer::toggleRightZPrepass() {
    rightEyeZPrepass = !rightEyeZPrepass;
}

void ReprojectiveStereoscopicRenderer::toggleDebugMode() {
    GLint l = GL.glGetUniformLocation( 	shaderProgram.programId(),"debugMode");
    int value = -1;
    GL.glGetUniformiv(shaderProgram.programId(),l,&value);
    shaderProgram.setUniformValue( "debugMode", value == 1 ? 0 : 1 );
    debugMode = value == 1 ? true : false;
}

void ReprojectiveStereoscopicRenderer::draw(Scene* s) {
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

    QVector3D originalCameraPosition = cameraPosition;
    QVector3D leftCameraPosition = originalCameraPosition - (right * eyeSeparation / 2.0f);
    QVector3D rightCameraPosition = originalCameraPosition + (right * eyeSeparation / 2.0f);

    //position the viewports on the screen somehow
    int w = viewport[2];
    int h = viewport[3];
    //depth difference threshold to recognize non-reusable fragments
    //shaderProgram.setUniformValue( "depthDifThreshold", eyeSeparation );
    //qDebug() <<  eyeSeparation /  FarClippingPlane / 1000.0f;
    shaderProgram.setUniformValue( "depthThreshold",eyeSeparation / FarClippingPlane / 10.0f );
    //set projection matrix (which is the same for both eyes)
    shaderProgram.setUniformValue( "P", projection );
    shaderProgram.setUniformValue( "height", (float)h  );
    shaderProgram.setUniformValue( "width", (float)w );
    GL.glGetIntegerv(GL_VIEWPORT,viewport);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[0]); //left
    //draw to attachments
    GLenum drawBufs[3] = {GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2};
    GL.glDrawBuffers(3,drawBufs);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //draw left eye
    shaderProgram.setUniformValue("eyeIndex",0);
    shaderProgram.setUniformValue("eyeSeparation",eyeSeparation);
    GL.glViewport( 0, 0,w,h );
    setCameraPosition(leftCameraPosition);
    //set right camera position as well, used in fragment shader
    shaderProgram.setUniformValue("rightCameraWorldPos",rightCameraPosition);

    shaderProgram.setUniformValue( "V", viewLeft );
    //first draw opaque. store depth values in exchange buffer
    GL.glDisable(GL_BLEND);
    //zprepass
    if(leftEyeZPrepass) {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LEQUAL);

        GL.glDrawBuffer(GL_NONE);
        zPrepassShaderProgram.bind();
        s->bind(&zPrepassShaderProgram);
        s->draw(&zPrepassShaderProgram,viewLeft,projection, OPAQUE);

        shaderProgram.bind();
        s->bind(&shaderProgram);
        GL.glDrawBuffers(3,drawBufs);
    } else {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LESS);
    }
    s->draw(&shaderProgram,viewLeft,projection, OPAQUE);


    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    //draw to attachments
    GL.glDrawBuffers(3,drawBufs);
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    //bind "exchange" buffer for reading from the LEFT fbo
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Exchange]);
    shaderProgram.setUniformValue("exchangeBufferSampler" , 0);
    //bind left image color buffer for reading
    GL.glActiveTexture(GL_TEXTURE1);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Color]);
    shaderProgram.setUniformValue("leftImageSampler" , 1);
    //bind left image depth buffer
    GL.glActiveTexture(GL_TEXTURE2);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Depth]);
    shaderProgram.setUniformValue("exchangeBuffer2Sampler" , 2);

    //GL.glClampColor(GL_CLAMP_READ_COLOR,GL_FALSE); //avoid clamping

    //draw right eye
    //GL.glDrawBuffer(GL_COLOR_ATTACHMENT1); //draw into right color buffer
    shaderProgram.setUniformValue("eyeIndex",1);
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
        s->draw(&zPrepassShaderProgram,viewRight,projection, OPAQUE);

        shaderProgram.bind();
        s->bind(&shaderProgram);
        GL.glDrawBuffers(3,drawBufs);
    } else {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LESS);
    }

    //reproject fragments. if not reprojectable, give them color value (0,0,0,0)
    s->draw(&shaderProgram,viewRight,projection, OPAQUE );

    //read color buffer, write into stencil buffer: 0 if color is (0,0,0,0), 1 else
    //-> color buffer gets copied into stencil buffer.
    copyColorBufToStencilBuf.bind();
    plane->bind(&copyColorBufToStencilBuf);
    QMatrix4x4 originView;
    originView.setToIdentity();
    originView.lookAt(
                QVector3D(0.0f, 0.0f, 0.0f),    // Camera Position
                QVector3D(0.0f, 0.0f, -1.0f),    // Point camera looks towards
                QVector3D(0.0f, 1.0f, 0.0f));
    copyColorBufToStencilBuf.setUniformValue( "height", (float)h  );
    copyColorBufToStencilBuf.setUniformValue( "width", (float)w );
    //bind left image color buffer for reading
    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[1][Color]);
    copyColorBufToStencilBuf.setUniformValue("rightImageColor" , 0);
    GL.glDrawBuffer(GL_COLOR_ATTACHMENT0);
    //draw stencil mask
    GL.glEnable(GL_STENCIL_TEST);
    GL.glDepthMask(GL_FALSE);
    GL.glStencilFunc(GL_NEVER, 1, 0xFF);
    GL.glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
    GL.glStencilMask(0xFF);
    GL.glClear(GL_STENCIL_BUFFER_BIT);
    plane->draw(&copyColorBufToStencilBuf,originView,projection,OPAQUE);
    GL.glDepthMask(GL_TRUE);
    GL.glDrawBuffers(3,drawBufs);
    shaderProgram.bind();
    GL.glDepthMask(GL_TRUE);
    GL.glStencilMask(0x00);
    GL.glStencilFunc(GL_EQUAL, 0, 0xFF); //draw where stencil is 1
    GL.glDepthFunc(GL_LEQUAL); //render pass in l.162 fills the depth buffer, so need less equal now.
    shaderProgram.setUniformValue("eyeIndex",0); //full render pass

    //full light calculation pass, but less costly because of early stencil test culling away a lot
    s->draw(&shaderProgram,viewRight,projection, OPAQUE );
    glDisable(GL_STENCIL_TEST);

    //transparent objects
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[0]); //left
    GL.glDrawBuffers(3,drawBufs);
    shaderProgram.setUniformValue("eyeIndex",0);
    GL.glViewport( 0, 0,w,h );
    setCameraPosition(leftCameraPosition);
    shaderProgram.setUniformValue( "V", viewLeft );
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewLeft,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);

    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    GL.glDrawBuffers(3,drawBufs);
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,w,h );
    setCameraPosition(rightCameraPosition);
    shaderProgram.setUniformValue( "V", viewRight );
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewRight,projection, TRANSPARENT);
    GL.glDisable(GL_BLEND);

    //blit framebuffer data to screen
    GL.glBindFramebuffer(GL_READ_FRAMEBUFFER,fbos[1]);
    GL.glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
    //GLenum status = GL.glGetError();
    //qDebug() << status;
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);//right camera
    GL.glBlitFramebuffer(0,0,w,h,
                         0,0,w,h, GL_COLOR_BUFFER_BIT,GL_NEAREST);
}

void ReprojectiveStereoscopicRenderer::initialize(int w, int h) {
    CanonicalMonoscopicRenderer::initialize();
    shaderProgram.setUniformValue("debugMode",0);
    debugMode = true;
    leftEyeZPrepass = false;
    rightEyeZPrepass = false;

    GL.glViewport( 0, 0,w,h );
    qDebug() << w << " " << h;
    //if the framebuffers got initialized already, deallocate the memory
    GL.glDeleteRenderbuffers(NumRenderbuffers,renderbuffers[0]);
    GL.glDeleteRenderbuffers(NumRenderbuffers,renderbuffers[1]);
    GL.glDeleteFramebuffers(NumFBOs,fbos);
    initializeFBO(0,w,h);
    initializeFBO(1,w,h);
    //qDebug() << "color: " << GL.glGetFragDataLocation(shaderProgram.programId(), "color");
    //qDebug() << "exchangeBuffer: " << GL.glGetFragDataLocation(shaderProgram.programId(), "exchangeBuffer");
    GL.glBindFramebuffer(GL_FRAMEBUFFER,0);


    copyColorBufToStencilBuf.removeAllShaders();
    QString vertexShaderPath = ":/simpleVert.glsl";
    QString fragmentShaderPath = ":/fragCopyColorToStencil.glsl";
    //compile shaders
    if ( !copyColorBufToStencilBuf.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath.toUtf8() ) ) {
        qDebug() << "ERROR (vertex shader):" << copyColorBufToStencilBuf.log();
    }
    if ( !copyColorBufToStencilBuf.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath.toUtf8() ) ) {
        qDebug() << "ERROR (fragment shader):" << copyColorBufToStencilBuf.log();
    }
    if ( !copyColorBufToStencilBuf.link() ) {
        qDebug() << "ERROR linking shader program:" << copyColorBufToStencilBuf.log();
    }
    copyColorBufToStencilBuf.bind();

    plane = new Scene("plane/plane.fbx");
    plane->load(&copyColorBufToStencilBuf);
    plane->bind(&copyColorBufToStencilBuf);

    shaderProgram.bind();


}
void ReprojectiveStereoscopicRenderer::initializeFBO(int fboIndex, int w , int h) {
    GL.glGenTextures(NumRenderbuffers,renderbuffers[fboIndex]);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Color]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Exchange]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,     GL_R8     ,w,h,0,     GL_RED    ,  GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Depth]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    GL.glTexImage2D(GL_TEXTURE_2D,0,    GL_DEPTH24_STENCIL8  ,w,h,0,   GL_DEPTH_STENCIL  , GL_FLOAT_32_UNSIGNED_INT_24_8_REV ,NULL); // https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml

    GL.glGenFramebuffers(1,&fbos[fboIndex]);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[fboIndex]);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderbuffers[fboIndex][Color],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,renderbuffers[fboIndex][Exchange],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,GL_TEXTURE_2D,renderbuffers[fboIndex][Depth],0);

    GLenum status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    qDebug() << status;

}
