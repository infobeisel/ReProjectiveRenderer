#include "canonicalmonoscopicrenderer.h"

CanonicalMonoscopicRenderer::CanonicalMonoscopicRenderer()
{
}
void CanonicalMonoscopicRenderer::setProjectionMatrix(QMatrix4x4 p) {projection = p;}
void CanonicalMonoscopicRenderer::setViewMatrix(QMatrix4x4 v) {view = v;}
void CanonicalMonoscopicRenderer::setCameraPosition(QVector3D p) {
    cameraPosition = p;
    shaderProgram.setUniformValue("cameraWorldPos",cameraPosition);
}

void CanonicalMonoscopicRenderer::setCameraOrientation(QQuaternion p) {
    cameraOrientation = p;
}

void CanonicalMonoscopicRenderer::draw(Scene* s) {

    // Clear color and depth buffers
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //zPrepass

    shaderProgram.setUniformValue("zPrepass",true);
    s->draw(&shaderProgram,view,projection, TRANSPARENT|OPAQUE);

    //first draw opaque, then transparent
    shaderProgram.setUniformValue("zPrepass",false);
    GL.glDisable(GL_BLEND);
    s->draw(&shaderProgram,view,projection, OPAQUE);
    GL.glEnable(GL_BLEND);
    GL.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    s->draw(&shaderProgram,view,projection, TRANSPARENT);

}

void CanonicalMonoscopicRenderer::initialize() {
    QString vertexShaderPath = ":/vertex.glsl";
    QString fragmentShaderPath = ":/fragment.glsl";
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

    //configure view & projection matrix
    view.setToIdentity();
    view.lookAt(
                QVector3D(0.0f, 0.0f, 0.0f),    // Camera Position
                QVector3D(0.0f, 0.0f, -1.0f),    // Point camera looks towards
                QVector3D(0.0f, 1.0f, 0.0f));   // Up vector

    float aspect = 4.0f/3.0f;
    projection.setToIdentity();
    projection.perspective(
                60.0f,          // field of vision
                aspect,         // aspect ratio
                0.3f,           // near clipping plane
                10000.0f);       // far clipping plane

    //will be modified later
    GL.glEnable(GL_DEPTH_TEST);
    GL.glDepthFunc(GL_LESS);
    //glDisable(GL_DEPTH_TEST);
    GL.glEnable(GL_CULL_FACE); //backface culling
    GL.glClearColor(.9f, .9f, .93f ,1.0f);

}
