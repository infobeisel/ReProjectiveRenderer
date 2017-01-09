#include "clienttexturearray.h"
#include <QDebug>
ClientTextureArray::ClientTextureArray(int twidth, int theight)
{
    textureNameMap = QMap<QString,ClientTexture*>();
    textures = QVector<ClientTexture*>();
    width = twidth;
    height = theight;
    textureUnitIndex = 0;
    textureArrayName = 0;

}
ClientTextureArray::~ClientTextureArray()
{
    textures = QVector<ClientTexture*>();
    for(int i= 0;i < textures.size(); i++) {
        delete textures[i];
    }
    textures.clear();
    QMap<QString,ClientTexture*>::const_iterator i = textureNameMap.constBegin();
    while (i != textureNameMap.constEnd()) {
        delete i.value();
        ++i;
    }
    textureNameMap.clear();
}
ClientTexture* ClientTextureArray::getTexture(QString toTextureName) {
    return textureNameMap[toTextureName];
}
ClientTexture* ClientTextureArray::getTexture(int toTextureIndex) {
    return textures[toTextureIndex];
}
int ClientTextureArray::getTextureUnitIndex() {return textureUnitIndex;}
GLuint ClientTextureArray::getServerTextureName() {return textureArrayName;}
int ClientTextureArray::getWidth() {return width;}
int ClientTextureArray::getHeight() {return height;}
int ClientTextureArray::getDepth() {return textures.size();}

void ClientTextureArray::addTexture(ClientTexture* tex) {

    //if not already added
    QString tname = tex->getName();
    if(textureNameMap.isEmpty() || textureNameMap.find(tname) == textureNameMap.end()) {
        int ind = textures.size();
        textures.push_back(tex);
        tex->setIndex(ind);
        textureNameMap[tex->getName()] = tex;
    }
}


void ClientTextureArray::setTextureUnitIndex(int i) {
    textureUnitIndex = i;
}

void ClientTextureArray::setServerTextureName(GLuint name) {
    textureArrayName = name;
}
