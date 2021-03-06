#ifndef SCENE_H
#define SCENE_H
#include "clienttexturearraymanager.h"
#include <memory>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

typedef unsigned int	materialClassifier;
#define TRANSPARENT				0x0001
#define OPAQUE   				0x0002


class Scene
{
    struct LightSource {
        QVector3D ambient;
        QVector3D diffuse;
        QVector3D specular;
        QVector3D position;
        QVector3D direction;
        float attenuationConstant;
        float attenuationLinear;
        float attenuationQuadratic;

    };

    struct Node {
        aiNode* assimpNode;
        int indexBufferOffset;
        int indexCount;
    };


public:
    Scene(QString fullPath);
    ~Scene();
    void load(QOpenGLShaderProgram *toProgram); //loads the model into client side ram and then into server side buffer objects
    void bind(QOpenGLShaderProgram *toProgram); //bind everything and set attribute pointers
    //draw this scene
    void draw(QOpenGLShaderProgram *withProgram, QMatrix4x4 viewMatrix, QMatrix4x4 projMatrix,materialClassifier materialTypes);

    const aiScene*  getAiScene();

private:
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer vertexBufferObject;
    QOpenGLBuffer normalBufferObject;
    QOpenGLBuffer tangentBufferObject;
    QOpenGLBuffer textureUVBufferObject;
    QOpenGLBuffer indexBufferObject;
    QString name; //name of the scene file
    QString basePath; //path to the scene file

    //vector holding the opaque nodes NOTE: rendering does not use this yet.
    QVector<Node> transparentNodes;
    QVector<Node> opaqueNodes;

  //  QElapsedTimer timer;


    QMap<QString,int> lightSourceNameToLightArrayIndex;
    ClientTextureArrayManager* textureManager;
    //the importer object and the scene from assimp
    const aiScene* s;
    int nodeCount; // the total count of aiNode s nodes in the scene
    Assimp::Importer importer;
};
#endif // SCENE_H
