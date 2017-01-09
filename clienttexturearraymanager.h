#ifndef CLIENTTEXTUREARRAYMANAGER_H
#define CLIENTTEXTUREARRAYMANAGER_H
#include "clienttexturearray.h"
#include <QString>
#include <QOpenGLFunctions>
#include <QImage>


class ClientTextureArrayManager
{
public:
    ClientTextureArrayManager();
    ~ClientTextureArrayManager();
    void addImage(QString name, QImage* timg);
    void loadToServer(QOpenGLFunctions* tgl);
    ClientTextureArray* getTextureArray(QString toTextureName);
private:
    QMap<QString,ClientTextureArray*> texResToArray;
    QOpenGLFunctions* gl;
    QVector<GLuint> texArrays;
};

#endif // CLIENTTEXTUREARRAYMANAGER_H
