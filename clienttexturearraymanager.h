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
    void addImage(QString name, QImage* timg);
    void loadToServer(QOpenGLFunctions* gl);
    ClientTextureArray* getTextureArray(QString toTextureName);
private:
    QMap<QString,ClientTextureArray*> texResToArray;
};

#endif // CLIENTTEXTUREARRAYMANAGER_H
