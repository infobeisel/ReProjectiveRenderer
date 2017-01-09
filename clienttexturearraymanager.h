#ifndef CLIENTTEXTUREARRAYMANAGER_H
#define CLIENTTEXTUREARRAYMANAGER_H
#include "clienttexturearray.h"
#include <QString>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <QImage>


class ClientTextureArrayManager
{
public:
    ClientTextureArrayManager();
    ~ClientTextureArrayManager();
    void addImage(QString name, QImage* timg);
    //only call this once to load all gathered texture data to the opengl server
    void loadToServer(QOpenGLFunctions* tgl);
    //prepares sampler with given sampler name to read from texture with given texture name
    void bindLoadedTexture(QString tName,const char* samplerName,const  char* arrayIndexName,QOpenGLShaderProgram* withProgram);
    ClientTextureArray* getTextureArray(QString toTextureName);
private:
    QMap<QString,ClientTextureArray*> texResToArray;
    QOpenGLFunctions* gl;
    QVector<GLuint> texArrays;
};

#endif // CLIENTTEXTUREARRAYMANAGER_H
