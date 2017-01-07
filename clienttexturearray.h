#ifndef CLIENTTEXTUREARRAY_H
#define CLIENTTEXTUREARRAY_H
#include "clienttexture.h"
#include <QMap>
#include <QVector>
#include <QOpenGLFunctions>

class ClientTextureArray
{
public:
    ClientTextureArray(int twidth, int theight);
    ClientTexture* getTexture(QString toTextureName);
    ClientTexture* getTexture(int toTextureIndex);
    void addTexture(ClientTexture* tex);
    int getWidth();
    int getHeight();
    int getDepth();
    int getTextureUnitIndex();
    GLuint getServerTextureName();
    void setTextureUnitIndex(int i);
    void setServerTextureName(GLuint name);
private:
    QMap<QString,ClientTexture*> textureNameMap;
    QVector<ClientTexture*> textures;
    int width;
    int height;
    int textureUnitIndex;
    GLuint textureArrayName;
};

#endif // CLIENTTEXTUREARRAY_H
