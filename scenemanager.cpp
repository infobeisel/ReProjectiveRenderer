#include "scenemanager.h"

SceneManager::SceneManager()
{

}

void SceneManager::loadScene(Scene* s) {
    if(!scenes.contains(s)) {
        scenes.push_back(s);
    }
    s->load();
    active = s;
}

Scene* SceneManager::getActive() {
    return active;
}

QVector<Scene*> SceneManager::getScenes() {
    return scenes;
}
