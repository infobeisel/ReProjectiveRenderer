#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H
#include <QVector>
#include "scene.h"

class SceneManager
{
public:
    SceneManager();
    void loadScene(Scene* s);
    Scene* getActive();
    QVector<Scene*> getScenes();
private:
    QVector<Scene*> scenes;
    Scene* active;

};

#endif // SCENEMANAGER_H
