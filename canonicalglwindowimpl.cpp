#include "openglfunctions.h"
#include "canonicalglwindowimpl.h"
#include "canonicalmonoscopicrenderer.h"
#include "canonicalstereoscopicrenderer.h"
#include "reprojectivestereoscopicrenderer.h"
#include "reprojectionerrorrenderer.h"
#include "scene.h"
#include <QCoreApplication>
#include <fstream>
#include <sstream>

#include <configuration.h>

CanonicalGLWindowImpl::CanonicalGLWindowImpl()
{

}
CanonicalGLWindowImpl::~CanonicalGLWindowImpl()
{
    delete scene;
    delete renderer;
}
void CanonicalGLWindowImpl::initializeGL() {

    //renderer = new CanonicalStereoscopicRenderer();
    timer.start();
    GL.glClear(0);

    if(Configuration::instance().RenderType == 0) {
        renderer = new CanonicalMonoscopicRenderer();
    } else if (Configuration::instance().RenderType == 1) {
        renderer = new CanonicalStereoscopicRenderer();
    } else if (Configuration::instance().RenderType == 2) {
        renderer = new ReprojectiveStereoscopicRenderer();
    } else if (Configuration::instance().RenderType == 3) {
        renderer = new ReprojectionErrorRenderer();
    }
    renderer->initialize();


    rerenderNonReprojectedPixels = false;
    pixelCountersEnabled = false;

    //load models
    std::ifstream in("scenelocation");
    std::string contents((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    //QString p = (QString::fromStdString(contents)).simplified(); //remove \n s etc.
    QStringList paths = QString::fromStdString(contents).split("\n");



    scene = new Scene(paths.at(0));
    scene->load(renderer->getShaderProgram());
    scene->bind(renderer->getShaderProgram());

    //shadows
    shadows.initialize(2048,2048);
    shadows.draw(scene);
     shadows.saveToImage("test.png");


    qDebug() << paths.at(0) << "  " << paths.at(1) << "  "<< paths.at(2) << "  "<< paths.at(3) ;
    if(paths.size() > 2) { //assume that second one has to be a scene containing camera positions for a camera tour
        camTour = new CameraTour(paths.at(1));
        camTour->load();
    }


    //movement
    lastCursorPos = QVector<int>();
    lastCursorPos.push_back(QCursor::pos().x());
    lastCursorPos.push_back(QCursor::pos().y());
    viewAngles = QVector<float>();
    viewAngles.push_back(0.0f);
    viewAngles.push_back(0.0f);
    cameraPosition = QVector3D(0.0f,0.0f,0.0f);
    moveDir= QVector3D(0.0f,0.0f,0.0f);

    //
    fpsLogger = CSVFileHandle<int>(3000);
    cameraAnimationTimeLogger = CSVFileHandle<float>(3000);
    pixelCountLogger = CSVFileHandle<float>(3000);
    pixelCounter = PixelCounter();

    time.start();
    guiUpdateTime.start();

    //register update event
    //QObject::connect(this,
    //                 SIGNAL(frameSwapped()),
    //                 this,
    //                 SLOT(update()));
}

void CanonicalGLWindowImpl::resizeGL(int w, int h) {
    GL.glViewport( 0, 0, w, h );

    //nProj.ortho(-(float)w/2.0f,(float)w/2.0f,-(float)h/2.0f,(float)h/2.0f,NearClippingPlane,FarClippingPlane);
    renderer->setProjectionMatrix(Configuration::instance().FoV, (float)w/(float)h, Configuration::instance().NearClippingPlane, Configuration::instance().FarClippingPlane);
    renderer->initialize(w,h);
}

void CanonicalGLWindowImpl::paintGL() {



    //manual movement
    //tell the shader the camera world pos
    renderer->setCameraPosition(cameraPosition);
    renderer->setCameraOrientation(cameraOrientation);
    QMatrix4x4 nView = QMatrix4x4();
    handleCursor(&nView);

    //animated movement
    float t =  (( time.elapsed() / 1000.0f) - Configuration::instance().CameraTourStartOffsetInSeconds) / Configuration::instance().CameraTourDurationInSeconds;
    if(camTour->isValid()) {
        if(t > 0.0f && t < 1.0f) { //camera animation has not ended yet
            QVector3D position;
            QVector3D dir;
            QVector3D up;
            camTour->getPositionForwardUp(t,position,dir,up);
            cameraPosition = position;
            //QVector3D cross = QVector3D::crossProduct(up,dir);
            //cameraOrientation = QQuaternion::fromDirection(cross.normalized(), up.normalized());
            cameraOrientation = QQuaternion::fromDirection(dir.normalized(), up.normalized());
            renderer->setCameraPosition(cameraPosition);
            renderer->setCameraOrientation(cameraOrientation);
        } else if(t > 1.0f){
            camTour->setValid(false); //end animation
            fpsLogger.flush("fpslog " + renderer->configTags());
            cameraAnimationTimeLogger.flush("frametimelog " + renderer->configTags());
            pixelCountLogger.flush("reprojectedPixelCountlog " + renderer->configTags());
            pixelCounter.saveReadImage("image");
            GLuint reprojectedImage = ((ReprojectiveStereoscopicRenderer*)renderer)->getLeftImage();
            pixelCounter.subtractGLTexture(reprojectedImage);
            //pixelCounter.readFromGLTexture(reprojectedImage);
            pixelCounter.saveReadImage("image2");

        }
    }


    //shadow map uniforms
    shadows.setShadowMapVariables(renderer->getShaderProgram());





    renderer->draw(scene);

    if(camTour->isValid() && t > 0.0f && t < 1.0f) {
        if(pixelCountersEnabled) {
            //count reprojected pixels
            GLuint reprojectedImage = ((ReprojectiveStereoscopicRenderer*)renderer)->getRightImage();
            pixelCounter.readFromGLTexture(reprojectedImage);
            if(rerenderNonReprojectedPixels) { //if they are rerendered measure the reprojection errror,
                GLuint leftImage = ((ReprojectiveStereoscopicRenderer*)renderer)->getLeftImage();
                //pixelCounter.subtractGLTexture(leftImage);
                //float error = 1.0f - pixelCounter.countPixelsWithColorFraction(QColor(0,0,0,0));
                //pixelCountLogger.addValue(error);
                float ssd = pixelCounter.calculateSSDToGLTexture(leftImage);
                pixelCountLogger.addValue(ssd);

            } else { //count pixels which would be rerendered
                pixelCountLogger.addValue(pixelCounter.countPixelsWithColorFraction(QColor(0,255,0,255)));
            }

        }
    }

     int msToWait = (1 - timer.elapsed()) < 0 ? 0 :(1 - timer.elapsed())  ;
     #ifdef Q_OS_WIN
         Sleep(uint(msToWait));
     #else
         struct timespec ts = { msToWait / 1000, (msToWait % 1000) * 1000 * 1000 };
         nanosleep(&ts, NULL);
     #endif


    auto fps = (int)(1000.0f / ( (float)timer.elapsed() + 0.0001f));
    if(camTour->isValid() && t > 0.0f && t < 1.0f) {
        fpsLogger.addValue(fps);
        cameraAnimationTimeLogger.addValue (t);
    }

    if(    guiUpdateTime.elapsed() > 200) {
        std::stringstream ss;
        ss << renderer->configTags().toStdString();
        ss << " FPS: " << fps;
        setTitle(ss.str().c_str());
        guiUpdateTime.start();

    }



    timer = QElapsedTimer();
    timer.start();

    //trigger an update so that this function gets called the next frame again
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));

    //qDebug() << (1000.0f / (float)timer.elapsed());
        //register update



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
    cameraPosition += moveDir.z() * ((float)timer.elapsed() / 1000.0f) * (shiftKeyHeld ? 0.1f : 1.0f) * (spaceKeyHeld ? 0.1f : 1.0f) *  forward  ;
    cameraPosition += moveDir.x() * ((float)timer.elapsed() / 1000.0f) * (shiftKeyHeld ? 0.1f : 1.0f) * (spaceKeyHeld ? 0.1f : 1.0f) * right  ;

    cameraOrientation = qx * qy;

    affect->setToIdentity();
    affect->lookAt(cameraPosition,cameraPosition + forward,up);

    QCursor::setPos(x() +  width() / 2,y() + height() / 2); //reset the cursor to center of screen
    lastCursorPos[0] = QCursor::pos().x();
    lastCursorPos[1] = QCursor::pos().y();
}

void CanonicalGLWindowImpl::keyPressEvent(QKeyEvent *ev) {


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
        case Qt::Key_Shift:
        shiftKeyHeld = true;
        break;
        case Qt::Key_Space:
        spaceKeyHeld = true;
        break;
        case Qt::Key_Up:
        if(shiftKeyHeld)
             renderer->setNormalizedEyeSeparation(renderer->getNormalizedEyeSeparation() + ((float)timer.elapsed() / 1000.0f));
        break;
        case Qt::Key_Down:
        if(shiftKeyHeld)
             renderer->setNormalizedEyeSeparation(renderer->getNormalizedEyeSeparation() - ((float)timer.elapsed() / 1000.0f));
        break;
        default:
        break;
    }
    float tspeed = 100.1f;
    moveDir += t * tspeed ;
}

void CanonicalGLWindowImpl::keyReleaseEvent(QKeyEvent *ev) {
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
        case Qt::Key_Shift:
        shiftKeyHeld = false;
        break;
        case Qt::Key_Space:
        spaceKeyHeld = false;
        break;
        case Qt::Key_F1:
        renderer->toggleDebugMode();
        rerenderNonReprojectedPixels = !rerenderNonReprojectedPixels;
        renderer->setProjectionMatrix(Configuration::instance().FoV, (float)width()/(float)height(), Configuration::instance().NearClippingPlane, Configuration::instance().FarClippingPlane);
        break;
        case Qt::Key_F2:
        pixelCountersEnabled = !pixelCountersEnabled;
        break;
        case Qt::Key_F3:
        (renderer)->toggleZPrepass();
        break;
        default:
        break;
    }
    float tspeed = 100.1f;
    moveDir -= t * tspeed ;
}
