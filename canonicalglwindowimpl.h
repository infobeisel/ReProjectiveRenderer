#ifndef CANONICALGLWINDOWIMPL_H
#define CANONICALGLWINDOWIMPL_H
#include <QOpenGLWindow>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include "scenemanager.h"
#include <QKeyEvent>
#include "canonicalmonoscopicrenderer.h"
#include "canonicalstereoscopicrenderer.h"

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
    void paintGL() ;
    //initializes opengl and loads shaders and loads the scene file according to the file "scenelocation" which should be
    //located in the execution directoy
    void initializeGL() ;
    void resizeGL(int w, int h) ;
private:
    CanonicalStereoscopicRenderer renderer;

    //camera movement
    QVector3D cameraPosition;
    QVector3D moveDir; //Vector that shows into the world coordinate direction (depending to wasd keys), where to move the camera
    QVector<int> lastCursorPos;//member to calculate the mouse movement since last frame
    QVector<float> viewAngles; //x-axis and y-axis angles in degrees
    //the scene to be rendered
    Scene* scene;

    void handleCursor(QMatrix4x4* affect);//translates cursor movement into view matrix updates
};
#endif // CANONICALGLWINDOWIMPL_H
