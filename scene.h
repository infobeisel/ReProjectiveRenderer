#ifndef SCENE_H
#define SCENE_H
#include "clienttexturearraymanager.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Scene
{
public:
    Scene(QString fullPath,QOpenGLFunctions* tgl);
    ~Scene();
    void load(); //loads the model into client side ram and then into server side buffer objects
    void bind(QOpenGLShaderProgram *toProgram); //bind everything and set attribute pointers
    //draw this scene
    void draw(QOpenGLShaderProgram *withProgram, QMatrix4x4 viewMatrix, QMatrix4x4 projMatrix);
private:
    QOpenGLFunctions* gl;//reference to the (already initialized! when this is constructed) OpenGl functions
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer vertexBufferObject;
    QOpenGLBuffer normalBufferObject;
    QOpenGLBuffer textureUVBufferObject;
    QOpenGLBuffer indexBufferObject;
    QString name; //name of the scene file
    QString basePath; //path to the scene file

    ClientTextureArrayManager textureManager;
    //the importer object and the scene from assimp
    const aiScene* s;
    Assimp::Importer importer;
};
#endif // SCENE_H
