#include "canonicalmonoscopicrenderer.h"
#include <QFile>
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

    QVector3D forward = cameraOrientation.rotatedVector (QVector3D(0.0,0.0,-1.0));
    QVector3D up = cameraOrientation.rotatedVector (QVector3D(0.0,1.0,0.0));

    view.setToIdentity();
    view.lookAt(cameraPosition,cameraPosition + forward,up);

    //first draw opaque, then transparent
    shaderProgram.setUniformValue("eyeIndex",0);
    shaderProgram.setUniformValue( "V", view );
    s->draw(&shaderProgram,view,projection, OPAQUE);

    //first draw opaque, then transparent
    shaderProgram.setUniformValue("eyeIndex",0);
    shaderProgram.setUniformValue( "V", view );
    s->draw(&shaderProgram,view,projection, TRANSPARENT);


}
void CanonicalMonoscopicRenderer::initialize(int w, int h) {
    initialize();
}

void CanonicalMonoscopicRenderer::initialize() {

    GL.glUseProgram(0);
    zPrepassShaderProgram.removeAllShaders();
    QString vertexShaderPath = ":/simpleVert.glsl";
    //compile shaders
    if ( !zPrepassShaderProgram.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath.toUtf8() ) ) {
        qDebug() << "ERROR (vertex shader):" << zPrepassShaderProgram.log();
    }

    if ( !zPrepassShaderProgram.link() ) {
        qDebug() << "ERROR linking shader program:" << zPrepassShaderProgram.log();
    }
    zPrepassShaderProgram.bind();


    GL.glUseProgram(0);
    shaderProgram.removeAllShaders();


    vertexShaderPath = ":/vertex.glsl";

    //compile vertex shader
    if ( !shaderProgram.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath.toUtf8() ) ) {
        qDebug() << "ERROR (vertex shader):" << shaderProgram.log();
    }


    QVector<QString> fragmentShaderPaths = {
        ":/shadowMappingShaders/Texture.glsl",
        ":/shadowMappingShaders/Sampling.glsl",
        ":/shadowMappingShaders/Shadow_Use_PCF.glsl",
        ":/fragmentVariables.glsl",
        ":/fragment.glsl"
    };
    //QString fragSourceCode = QString();
    foreach (QString file , fragmentShaderPaths) {
        QFile f(file);
        if (!f.open(QFile::ReadOnly | QFile::Text)) break;
        QTextStream in(&f);
        QString shaderCode =  in.readAll();
        //static shader compilation dependent on extension availability
        if(!GL_HasStencilTexturingExt) shaderCode.remove((const QString) QString::fromStdString("#define HasStencilTexturingExt"),Qt::CaseSensitive);
        if ( !shaderProgram.addShaderFromSourceCode( QOpenGLShader::Fragment,shaderCode) ) {
            qDebug() << "ERROR (fragment shader):" << shaderProgram.log();
        }
        f.close();
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
