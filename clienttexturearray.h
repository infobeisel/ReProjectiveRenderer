#ifndef CLIENTTEXTUREARRAY_H
#define CLIENTTEXTUREARRAY_H
#include "clienttexture.h"
#include <QMap>
#include <QVector>
#include <QOpenGLFunctions_4_1_Core>
class ClientTextureArray
{
public:
    ClientTextureArray(int twidth, int theight, int tformat);
    ~ClientTextureArray();
    ClientTexture* getTexture(QString toTextureName);
    ClientTexture* getTexture(int toTextureIndex);
    void addTexture(ClientTexture* tex);
    int getWidth();
    int getHeight();
    int getDepth();
    int getFormat();
    void setFormat(int f);
    int getTextureUnitIndex();
    GLuint getServerTextureName();
    void setTextureUnitIndex(int i);
    void setServerTextureName(GLuint name);
private:
    QMap<QString,ClientTexture*> textureNameMap;
    QVector<ClientTexture*> textures;
    int width;
    int height;
    int format;
    int textureUnitIndex;
    GLuint textureArrayName;
};

#endif // CLIENTTEXTUREARRAY_H
