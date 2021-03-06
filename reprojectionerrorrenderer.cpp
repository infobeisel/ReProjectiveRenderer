#include "reprojectionerrorrenderer.h"
#include "configuration.h"


QString ReprojectionErrorRenderer::configTags() {
    std::stringstream ss;
    ss << "ReprojectionErrorRenderer, Debug Mode " << (debugMode ? "true" : "false") << ", Z Prepass " << (zPrepass ? "true" : "false")  ;
    return QString::fromStdString(ss.str());
}

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

    //Reprojection matrix
    QMatrix4x4 reprojection;
    reprojection.setToIdentity();

    bool inv = true;
    //reprojection = leftProjection * viewLeft * viewRight.inverted(&inv) * rightProjection.inverted(&inv);

    //extract a_l,a_r,b_l,b_r,e,f,d, from projection matrices
    QVector4D firstRowLeft = leftProjection.row(0);
    QVector4D firstRowRight = rightProjection.row(0);
    QVector4D thirdRow = projection.row(2);

    float f = -1;
    float d = thirdRow.z();
    float e = thirdRow.w();
    float a = firstRowLeft.x();
    float b_l = firstRowLeft.z();
    float b_r = firstRowRight.z();

    QVector4D firstRowR = QVector4D(
                1.0,
                0.0f,
                 eyeSeparation * a  / e,
                ( (-b_r) / f ) - (( d * eyeSeparation * a) / (f * e)) + ( b_l / f)
                );
    reprojection.setRow(0,firstRowR);
    shaderProgram.setUniformValue( "R", reprojection );







    QVector3D originalCameraPosition = cameraPosition;
    QVector3D leftCameraPosition = originalCameraPosition - (right * eyeSeparation / 2.0f);
    QVector3D rightCameraPosition = originalCameraPosition + (right * eyeSeparation / 2.0f);

    //position the viewports on the screen somehow
    int w = viewport[2];
    int h = viewport[3];
    //depth difference threshold to recognize non-reusable fragments
    //shaderProgram.setUniformValue( "depthDifThreshold", eyeSeparation );
    //qDebug() <<  eyeSeparation /  FarClippingPlane / 1000.0f;
    shaderProgram.setUniformValue( "depthThreshold",Configuration::instance().DepthThreshold );

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
    //set projection matrix
    shaderProgram.setUniformValue( "P", projection );

    //first draw opaque. store depth values in exchange buffer
    GL.glDisable(GL_BLEND);
    //zprepass
    if(zPrepass) {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LEQUAL);

        GL.glDrawBuffer(GL_NONE);
        zPrepassShaderProgram.bind();
        s->bind(&zPrepassShaderProgram);
        s->draw(&zPrepassShaderProgram,viewLeft,leftProjection, OPAQUE);

        shaderProgram.bind();
        s->bind(&shaderProgram);
        GL.glDrawBuffers(3,drawBufs);
    } else {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LESS);
    }
    s->draw(&shaderProgram,viewLeft,leftProjection, OPAQUE);


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
    //set projection matrix
    shaderProgram.setUniformValue( "P", rightProjection );

    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,w,h );

    setCameraPosition(rightCameraPosition);
    shaderProgram.setUniformValue( "V", viewRight );
    GL.glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    //zprepass
    if(zPrepass) {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LEQUAL);

        GL.glDrawBuffer(GL_NONE);
        zPrepassShaderProgram.bind();
        s->bind(&zPrepassShaderProgram);
        s->draw(&zPrepassShaderProgram,viewRight,rightProjection, OPAQUE);

        shaderProgram.bind();
        s->bind(&shaderProgram);
        GL.glDrawBuffers(3,drawBufs);
    } else {
        GL.glEnable(GL_DEPTH_TEST);
        GL.glDepthFunc(GL_LESS);
    }

    //reproject fragments. if not reprojectable, give them color value (0,0,0,0)
    s->draw(&shaderProgram,viewRight,rightProjection, OPAQUE );

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
    if(!debugMode) s->draw(&shaderProgram,viewRight,rightProjection, OPAQUE );
    glDisable(GL_STENCIL_TEST);

    //transparent objects
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[0]); //left
    GL.glDrawBuffers(3,drawBufs);
    shaderProgram.setUniformValue("eyeIndex",0);
    GL.glViewport( 0, 0,w,h );

    setCameraPosition(leftCameraPosition);
    shaderProgram.setUniformValue( "P", leftProjection );
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewLeft,leftProjection, TRANSPARENT);
    GL.glDisable(GL_BLEND);

    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[1]); //right
    GL.glDrawBuffers(3,drawBufs);
    shaderProgram.setUniformValue("eyeIndex",1);
    GL.glViewport( 0, 0,w,h );

    setCameraPosition(rightCameraPosition);
    shaderProgram.setUniformValue( "P", rightProjection );
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,viewRight,rightProjection, TRANSPARENT);
    GL.glDisable(GL_BLEND);







    //now draw right again, but on the left image
        GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[0]); //right
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
        if(zPrepass) {
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
            GL.glBlitFramebuffer(0,0,w, h,
            0,0,w, h, GL_COLOR_BUFFER_BIT,GL_NEAREST);



}


void ReprojectionErrorRenderer::setProjectionMatrix(float fov,float aspect, float near, float far) {

    projection = QMatrix4x4();
    projection.setToIdentity();



    qreal radians = (fov ) * M_PI / 180.0f;
    float eyeSeparation = Configuration::instance().MaxEyeSeparation * normalizedEyeSeparation;
    float nearWidth =  qTan(radians / 2.0f) * 2.0f * near;

    projection.frustum(-nearWidth/2.0f, nearWidth/2.0f, -nearWidth / (aspect * 2.0f), nearWidth /(aspect * 2.0f), near, far);

    orthoProjection = QMatrix4x4();
    orthoProjection.setToIdentity();
    orthoProjection.ortho(-nearWidth/2.0f, nearWidth/2.0f, -nearWidth / (aspect * 2.0f), nearWidth /(aspect * 2.0f), near, far);


    //derive left and right eye projection matrices
    leftProjection = QMatrix4x4();
    rightProjection = QMatrix4x4();
    leftProjection = projection;
    rightProjection = projection;

    /*
     * set 2n/r-l and r+l/r-l
     *
     * */
    QVector4D firstRowLeft = leftProjection.row(0);
    QVector4D firstRowRight = rightProjection.row(0);
    firstRowLeft.setZ(   eyeSeparation /  nearWidth);
    firstRowRight.setZ(   -eyeSeparation / nearWidth);
    leftProjection.setRow(0,firstRowLeft);
    rightProjection.setRow(0,firstRowRight);

    qDebug() << "near plane width: " << nearWidth ;
}
