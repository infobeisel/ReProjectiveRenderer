#ifndef CANONICALGLWINDOWIMPL_H
#define CANONICALGLWINDOWIMPL_H
#include <QOpenGLWindow>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include "scenemanager.h"
#include <QKeyEvent>
#include "renderer.h"
#include "canonicalstereoscopicrenderer.h"
#include <QElapsedTimer>
#include "utils/cameratour.h"
#include <QTime>
#include "utils/csvfilehandle.h"
#include "utils/pixelcounter.h"
#include "shadowmapgenerator.h"
#define CameraTourDurationInSeconds 20.0f
#define NearClippingPlane 0.3f
class CanonicalGLWindowImpl : public QOpenGLWindow
{
public:
    CanonicalGLWindowImpl();
    ~CanonicalGLWindowImpl();
    //both affect the view matrix with translations for movement
    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;
protected:
    //
    void paintGL() override;
    //initializes opengl and loads shaders and loads the scene file according to the file "scenelocation" which should be
    //located in the execution directoy
    void initializeGL() ;
    void resizeGL(int w, int h) ;
private:
    Renderer* renderer;

    //camera movement
    QVector3D cameraPosition;
    QQuaternion cameraOrientation;
    QVector3D moveDir; //Vector that shows into the world coordinate direction (depending to wasd keys), where to move the camera
    QVector<int> lastCursorPos;//member to calculate the mouse movement since last frame
    QVector<float> viewAngles; //x-axis and y-axis angles in degrees
    //camera move animation
    CameraTour* camTour;


    //the scene to be rendered
    Scene* scene;

    ShadowMapGenerator shadows;


    QElapsedTimer timer;
    QTime time;
    CSVFileHandle<int> fpsLogger;
    CSVFileHandle<float> cameraAnimationTimeLogger;
    PixelCounter pixelCounter;
    bool shiftKeyHeld;
    bool spaceKeyHeld;

    void handleCursor(QMatrix4x4* affect);//translates cursor movement into view matrix updates
};
#endif // CANONICALGLWINDOWIMPL_H
