#ifndef CANONICALGLWINDOWIMPL_H
#define CANONICALGLWINDOWIMPL_H
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include "scenemanager.h"
#include <QKeyEvent>
class CanonicalGLWindowImpl : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
    CanonicalGLWindowImpl();

    //both affect the view matrix with translations
    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;

protected:
    void paintGL() ;
    void initializeGL() ;
    void resizeGL(int w, int h) ;
private:
    //gl attributes
    QOpenGLShaderProgram shaderProgram;
    QMatrix4x4 view;
    QMatrix4x4 projection;

    //camera movement
    QVector3D cameraPosition;
    QVector3D moveDir; //Vector that shows into the world coordinate direction (depending to wasd keys), where to move the camera
    int* lastCursorPos;
    float* viewAngles; //x-axis and y-axis angles in degrees

    Scene* scene;


    void handleCursor(QMatrix4x4* affect);//translates cursor movement into view matrix updates

};

#endif // CANONICALGLWINDOWIMPL_H
