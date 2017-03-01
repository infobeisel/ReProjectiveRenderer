#include "shadowmapgenerator.h"
#include "utils/assimpscenesearch.h"

ShadowMapGenerator::ShadowMapGenerator()
{
}
void ShadowMapGenerator::initialize(int w, int h)  {
    CanonicalMonoscopicRenderer::initialize();
    mapResolution[0] = w;
    mapResolution[1] = h;


    GL.glUseProgram(0);
    shaderProgram.removeAllShaders();
    QString vertexShaderPath = ":/simpleVert.glsl";
    QString fragmentShaderPath = ":/genShadowMapFrag.glsl";
    //compile shaders
    if ( !shaderProgram.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath.toUtf8() ) ) {
        qDebug() << "ERROR (vertex shader):" << shaderProgram.log();
    }
    if ( !shaderProgram.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath.toUtf8() ) ) {
        qDebug() << "ERROR (fragment shader):" << shaderProgram.log();
    }

    if ( !shaderProgram.link() ) {
        qDebug() << "ERROR linking shader program:" << shaderProgram.log();
    }
    shaderProgram.bind();

    //create fbo which holds the shadow map
    //if the framebuffers got initialized already, deallocate the memory
    GL.glDeleteRenderbuffers(NumRenderbuffers,renderbuffers[0]);
    GL.glDeleteRenderbuffers(NumRenderbuffers,renderbuffers[1]);
    GL.glDeleteFramebuffers(NumFBOs,fbos);

    initializeFBO(Shadow,w,h);
    GL.glDrawBuffer(GL_COLOR_ATTACHMENT0);

}

void ShadowMapGenerator::draw(Scene* s) {

    shaderProgram.bind();
    s->bind(&shaderProgram);

    //prepare fbo
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[Shadow]);
    GL.glViewport(0,0,mapResolution[0],mapResolution[1]);

    const aiScene* scene = s->getAiScene();
    qDebug() << "ShadowMapGenerator: draw call, go through lights and render depths for all directional lights";
    if(scene->HasLights()) {
        for(int i = 0; i < scene->mNumLights; i++) {
            aiLight* li = scene->mLights[i];
            if(li->mType == aiLightSource_DIRECTIONAL ) {
                qDebug() << "ShadowMapGenerator: directional light found";
                aiNode* found = AssimpSceneSearch::find(QString(li->mName.data),scene->mRootNode);
                aiVector3t<float> tPos = aiVector3t<float>(0.0f,0.0f,0.0f);
                aiVector3t<float> tScl = aiVector3t<float>(0.0f,0.0f,0.0f);
                aiQuaterniont<float> q = aiQuaterniont<float>();


                //in blender, the directional light is looking in -y direction when the transform rotation is the identity
                aiMatrix3x3 rotation = aiMatrix3x3(found->mTransformation);

                li->mDirection = rotation * aiVector3t<float>(0.0f,-1.0f,0.0f);
                li->mDirection.Normalize();/*
                aiVector3t<float> aiUp = rotation * aiVector3t<float>(0.0f,0.0f,1.0f);
                QVector3D up = QVector3D(aiUp[0],aiUp[1],aiUp[2]);*/


                tPos = found->mTransformation * tPos;
                QVector3D lightPosition = QVector3D( tPos[0], tPos[1],tPos[2]);
                QVector3D lightDirection = QVector3D(li->mDirection[0],li->mDirection[1],li->mDirection[2]).normalized();


                //up vector is not specified, so create one.
                QVector3D tPos2 = lightPosition + lightDirection;
                QVector3D up = QVector3D::crossProduct(lightPosition,tPos2);

                cameraPosition = lightPosition;





                GL.glClearDepth(1.0f);
                GL.glClear(GL_DEPTH_BUFFER_BIT);

                GL.glEnable(GL_POLYGON_OFFSET_FILL);
                GL.glPolygonOffset(2.0f,4.0f);

                qDebug()<< "light direction " << lightDirection;

                //configure view & projection matrix
                view.setToIdentity();
                view.lookAt(
                            lightPosition,    // Camera Position
                            lightPosition +  lightDirection,    // Point camera looks towards
                            up);   // Up vector

                projection.setToIdentity();
                projection.ortho(-mapResolution[0],mapResolution[0],-mapResolution[1],mapResolution[1],NearClippingPlane,FarClippingPlane);
                //projection.ortho(-200,200,-200,200,NearClippingPlane,FarClippingPlane);

                GL.glDrawBuffer(GL_COLOR_ATTACHMENT0);
                s->draw(&shaderProgram,view,projection, OPAQUE);

               GL.glDisable(GL_POLYGON_OFFSET_FILL);

               //only render from the first occuring
               break;
            }
        }
    }


}
void ShadowMapGenerator::setShadowMapVariables(QOpenGLShaderProgram* toShader) {
    //make shadow map accessble in shader
    GL.glActiveTexture(GL_TEXTURE3);
    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[0][Depth]);
    toShader->setUniformValue("ShadowMap" , 3);
    toShader->setUniformValue("ShadowCameraPosition",cameraPosition);
    QMatrix4x4 viewProj = projection * view;
    toShader->setUniformValue("ShadowCameraViewProjectionMatrix",viewProj);
    toShader->setUniformValue("ShadowMapSize",QVector2D((float)mapResolution[0],(float)mapResolution[1]));

}

void ShadowMapGenerator::saveToImage(QString path) {
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[Shadow]);

    QImage i( mapResolution[0],mapResolution[1],QImage::Format_Grayscale8 /*QImage::Format_RGBA8888*/ );
    GL.glReadBuffer(GL_COLOR_ATTACHMENT0);
    GL.glReadPixels(0,0,mapResolution[0],mapResolution[1],GL_DEPTH_COMPONENT/*GL_RGBA*/,GL_UNSIGNED_BYTE,i.bits());
    i.save(path);

}

void ShadowMapGenerator::initialize() {initialize(1024,1024);}

void ShadowMapGenerator::initializeFBO(int fboIndex, int w , int h) {
    GLenum status = GL.glGetError();

    GL.glGenTextures(NumRenderbuffers,renderbuffers[fboIndex]);


    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Color]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    GL.glBindTexture(GL_TEXTURE_2D,renderbuffers[fboIndex][Depth]);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Setting texture sampling:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    GL.glTexImage2D(GL_TEXTURE_2D,0,   GL_DEPTH_COMPONENT   ,w,h,0,  GL_DEPTH_COMPONENT  ,GL_UNSIGNED_BYTE/*GL_FLOAT*/,NULL);

    GL.glGenFramebuffers(1,&fbos[fboIndex]);
    GL.glBindFramebuffer(GL_FRAMEBUFFER,fbos[fboIndex]);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,renderbuffers[fboIndex][Color],0);
    GL.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,renderbuffers[fboIndex][Depth],0);


    status = GL.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    qDebug() << status;

    //will be modified later
    GL.glEnable(GL_DEPTH_TEST);
    GL.glDepthFunc(GL_LESS);
}
