#include "canonicalglwindowimpl.h"
#include "scene.h"
#include <QCoreApplication>
#include <fstream>

CanonicalGLWindowImpl::CanonicalGLWindowImpl()
{
}
CanonicalGLWindowImpl::~CanonicalGLWindowImpl()
{
    delete scene;
}
void CanonicalGLWindowImpl::initializeGL() {
    initializeOpenGLFunctions();
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); //backface culling
    glClearColor(.9f, .9f, .93f ,1.0f);


    //load models
    std::ifstream in("scenelocation");
    std::string contents((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    //QString p = (QString::fromStdString(contents)).simplified(); //remove \n s etc.
    QStringList paths = QString::fromStdString(contents).split("\n");


    scene = new Scene(paths.at(0),this);
    scene->load(&shaderProgram);
    scene->bind(&shaderProgram);

    //movement
    lastCursorPos = QVector<int>();
    lastCursorPos.push_back(QCursor::pos().x());
    lastCursorPos.push_back(QCursor::pos().y());
    viewAngles = QVector<float>();
    viewAngles.push_back(0.0f);
    viewAngles.push_back(0.0f);
    cameraPosition = QVector3D(0.0f,0.0f,0.0f);
    moveDir= QVector3D(0.0f,0.0f,0.0f);
}

void CanonicalGLWindowImpl::resizeGL(int w, int h) {
    glViewport( 0, 0, w, h );
    projection.setToIdentity();
    projection.perspective(60.0f, (float)w/h, 0.3f, 10000);
}

void CanonicalGLWindowImpl::paintGL() {
    // Clear color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //tell the shader the camera world pos
    shaderProgram.setUniformValue("cameraWorldPos",cameraPosition);
    glDisable(GL_BLEND);
    scene->draw(&shaderProgram,view,projection, OPAQUE);
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    scene->draw(&shaderProgram,view,projection, TRANSPARENT);

    handleCursor(&view);
    //trigger an update so that this function gets called the next frame again
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

void CanonicalGLWindowImpl::handleCursor(QMatrix4x4* affect) {

    float mousex = ((float) (QCursor::pos().x() - lastCursorPos[0]))  ;
    float mousey = ((float) (QCursor::pos().y() - lastCursorPos[1])) ;
    viewAngles[0] += mousex;
    viewAngles[0] = ( (int)viewAngles[0] % 360);
    viewAngles[1] += mousey;
    viewAngles[1] = ( (int)viewAngles[1] % 360);

    QVector3D up = QVector3D(0.0f,1.0f,0.0f);
    QVector3D forward = QVector3D(0.0f,0.0f,-1.0f);

    QQuaternion qy = QQuaternion::fromAxisAndAngle(up, -viewAngles[0]);
    forward = qy.rotatedVector(forward);        //new forward vector
    up = qy.rotatedVector(up);        //new up vector
    QVector3D right = QVector3D::crossProduct(forward,up);
    QQuaternion qx = QQuaternion::fromAxisAndAngle(right, -viewAngles[1]);
    forward = qx.rotatedVector(forward);        //new forward vector
    up = qx.rotatedVector(up);        //new up vector
    //move the camera
    cameraPosition += moveDir.z() * forward;
    cameraPosition += moveDir.x() * right;

    affect->setToIdentity();
    affect->lookAt(cameraPosition,cameraPosition + forward,up);

    QCursor::setPos(x() +  width() / 2,y() + height() / 2); //reset the cursor to center of screen
    lastCursorPos[0] = QCursor::pos().x();
    lastCursorPos[1] = QCursor::pos().y();
}

void CanonicalGLWindowImpl::keyPressEvent(QKeyEvent *ev) {
    float tspeed = 4.1f;//hardcoded movement speed
    QVector3D t = QVector3D(0.0f,0.0f,0.0f);
    switch(ev->key()) {
        case Qt::Key_W:
        t = QVector3D(0.0f,0.0f,1.0f);
        break;
        case Qt::Key_A:
        t = QVector3D(-1.0f,0.0f,0.0f);
        break;
        case Qt::Key_S:
        t = QVector3D(0.0f,0.0f,-1.0f);
        break;
        case Qt::Key_D:
        t = QVector3D(1.0f,0.0f,0.0f);
        break;
        default:
        break;
    }
    moveDir += t * tspeed;
}

void CanonicalGLWindowImpl::keyReleaseEvent(QKeyEvent *ev) {
    float tspeed = 4.1f;
    QVector3D t = QVector3D(0.0f,0.0f,0.0f);
    switch(ev->key()) {
        case Qt::Key_W:
        t = QVector3D(0.0f,0.0f,1.0f);
        break;
        case Qt::Key_A:
        t = QVector3D(-1.0f,0.0f,0.0f);
        break;
        case Qt::Key_S:
        t = QVector3D(0.0f,0.0f,-1.0f);
        break;
        case Qt::Key_D:
        t = QVector3D(1.0f,0.0f,0.0f);
        break;
        default:
        break;
    }
    moveDir -= t * tspeed;
}
