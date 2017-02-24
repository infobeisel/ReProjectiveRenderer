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
void CanonicalMonoscopicRenderer::setNormalizedEyeSeparation(float e) {}
float CanonicalMonoscopicRenderer::getNormalizedEyeSeparation() {return 0.0f;}
void CanonicalMonoscopicRenderer::draw(Scene* s) {

    // Clear color and depth buffers
    GL.glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgram.setUniformValue( "P", projection );

    //first draw opaque, then transparent
    shaderProgram.setUniformValue("eyeIndex",0);
    shaderProgram.setUniformValue( "V", view );
    s->draw(&shaderProgram,view,projection, OPAQUE);


}
void CanonicalMonoscopicRenderer::initialize(int w, int h) {
    initialize();
}

void CanonicalMonoscopicRenderer::initialize() {
    GL.glUseProgram(0);
    shaderProgram.removeAllShaders();


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

    shaderProgram.setUniformValue("debugMode",0);

    //configure view & projection matrix
    view.setToIdentity();
    view.lookAt(
                QVector3D(0.0f, 0.0f, 0.0f),    // Camera Position
                QVector3D(0.0f, 0.0f, -1.0f),    // Point camera looks towards
                QVector3D(0.0f, 1.0f, 0.0f));   // Up vector

    float aspect = 4.0f/3.0f;
    projection.setToIdentity();
    projection.perspective(
                FOV,          // field of vision
                aspect,         // aspect ratio
                NearClippingPlane,           // near clipping plane
                FarClippingPlane);       // far clipping plane

    //will be modified later
    GL.glEnable(GL_DEPTH_TEST);
    GL.glDepthFunc(GL_LESS);
    //glDisable(GL_DEPTH_TEST);
    GL.glEnable(GL_CULL_FACE); //backface culling
    GL.glClearColor(.9f, .9f, .93f ,1.0f);

}
