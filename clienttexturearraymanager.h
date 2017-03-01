#ifndef CLIENTTEXTUREARRAYMANAGER_H
#define CLIENTTEXTUREARRAYMANAGER_H
#include "clienttexturearray.h"
#include <QString>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>

#include <QImage>

//offset for possible use of first 3 texture units by the renderer for fbos
#define TEXTURE_UNIT_OFFSET 5

class ClientTextureArrayManager
{
public:
    ClientTextureArrayManager();
    ~ClientTextureArrayManager();
    void addImage(QString name, QImage* timg);
    //only call this once to load all gathered texture data to the opengl server
    void loadToServer();
    //prepares sampler with given sampler name to read from texture with given texture name
    void bindLoadedTexture(QString tName,const char* samplerName,const  char* arrayIndexName,QOpenGLShaderProgram* withProgram);
    ClientTextureArray* getTextureArray(QString toTextureName);
private:
    QMap<QString,ClientTextureArray*> texResToArray;
    QVector<GLuint> texArrays;
};

#endif // CLIENTTEXTUREARRAYMANAGER_H
