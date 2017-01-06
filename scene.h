#ifndef SCENE_H
#define SCENE_H
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

class Scene
{
public:
    Scene(QString fullPath,QOpenGLFunctions* tgl);
    ~Scene();
    void load();
    void bind(QOpenGLShaderProgram *toProgram);
    void draw(QOpenGLShaderProgram *withProgram, QMatrix4x4 viewMatrix, QMatrix4x4 projMatrix);
private:
    QOpenGLFunctions* gl;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer vertexBufferObject;
    QOpenGLBuffer normalBufferObject;
    QOpenGLBuffer textureUVBufferObject;
    QOpenGLBuffer indexBufferObject;
    QString name;
    QString basePath;

    GLuint textureArray;
    int* texRes;
    QMap<QString, int> texturePathToArrayIndex;

    const aiScene* s;
    Assimp::Importer importer;




};

#endif // SCENE_H
