#include "clienttexturearraymanager.h"
#include <QList>
#include <QDebug>
ClientTextureArrayManager::ClientTextureArrayManager()
{
}

ClientTextureArrayManager::~ClientTextureArrayManager()
{
    //delete the client texture arrays
    QMap<QString,ClientTextureArray*>::const_iterator i = texResToArray.constBegin();
    while (i != texResToArray.constEnd()) {
        delete i.value();
        ++i;
    }
    //delete opengl server memory
    GL.glDeleteTextures(texArrays.size(),texArrays.data());

}
void ClientTextureArrayManager::addImage(QString tname, QImage* timg) {
    ClientTexture* newTex = new ClientTexture(tname, timg);
    //key: concatenate width and height
    QString res = QString::number(newTex->width()) + QString::number(newTex->height()) + QString::number(timg->format());
    //if no texture stack with this res exists, make a new one
    if(texResToArray.find(res) == texResToArray.end()) {
        ClientTextureArray* tar = new ClientTextureArray(newTex->width(),newTex->height(),timg->format());
        texResToArray[res] = tar;

    }
    //add the new texture to a texture array
    texResToArray[res]->addTexture(newTex);
}
/**
 * @brief ClientTextureArrayManager::getTextureArray < O(TextureArrayCount)
 * @param texture name to search
 * @return the ClientTextureArray which holds the texture with the given name
 */
ClientTextureArray* ClientTextureArrayManager::getTextureArray(QString toTextureName) {
    QMap<QString,ClientTextureArray*>::const_iterator i = texResToArray.constBegin();
    while (i != texResToArray.constEnd()) {
        ClientTextureArray* val = i.value();
        if(val->getTexture(toTextureName) != 0) {
            return val;
        }
        ++i;
    }
    return 0; //not yet added
}

void ClientTextureArrayManager::loadToServer() {
    int texArrayCount = texResToArray.size(); // texture array count to create
    texArrays = QVector<GLuint>(texArrayCount);
    GL.glGenTextures(texArrayCount,texArrays.data());
    QList<ClientTextureArray*> clientArrays = texResToArray.values();

    for(int i = 0 ; i < texArrayCount; i++) {

        ClientTextureArray* clientArray = clientArrays.at(i);
        GLint texArray = texArrays[i];
        int textureUnitIndex = GL_TEXTURE0 + TEXTURE_UNIT_OFFSET + i;
        clientArray->setServerTextureName(texArray);
        clientArray->setTextureUnitIndex(textureUnitIndex);
        GL.glActiveTexture(textureUnitIndex);
        GL.glBindTexture(GL_TEXTURE_2D_ARRAY,texArray);

        GLuint format = GL_BGRA;
        GLuint internalFormat = GL_RGBA;
        if(clientArray->getFormat() == QImage::Format_Grayscale8) {
            format = GL_RED;
            internalFormat = GL_RED;

        }


        GL.glTexImage3D (GL_TEXTURE_2D_ARRAY, 0,  internalFormat , clientArray->getWidth(), clientArray->getHeight(),
            clientArray->getDepth(), 0, format, GL_UNSIGNED_BYTE, 0);

        for(int imgi = 0; imgi<clientArray->getDepth(); imgi++) { //add each texture to the texture array
            GLenum t_target = GL_TEXTURE_2D_ARRAY;
            //https://www.opengl->org/sdk/docs/man2/xhtml/glTexSubImage3D.xml
            /*
             * insert the layer in zoffset (5th param) and not in depth (8th param, insert 1 there).
             * zoffset means layer, depth (probably) means mip map level(?)
             * */
            ClientTexture* clientTex = clientArray->getTexture(imgi);
            GL.glTexSubImage3D(t_target,
                            0,
                            0,0,imgi,
                            clientTex->width(),clientTex->height(),1,
                            format,         // format
                            GL_UNSIGNED_BYTE, // type
                            clientTex->image()->bits());
            GL.glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            GL.glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            GL.glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_REPEAT);
            GL.glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);
        }
        GL.glBindTexture(GL_TEXTURE_2D_ARRAY, 0); //unbind

    }
}

void ClientTextureArrayManager::bindLoadedTexture(QString tName,const char* samplerName,const char* arrayIndexVariableName, QOpenGLShaderProgram* withProgram) {


    ClientTextureArray* arr = getTextureArray(tName);
    if(arr != 0) {
        ClientTexture* tex = arr->getTexture(tName);
        GL.glActiveTexture(arr->getTextureUnitIndex());
        GL.glBindTexture(GL_TEXTURE_2D_ARRAY,arr->getServerTextureName());
        withProgram->setUniformValue(samplerName , arr->getTextureUnitIndex() - GL_TEXTURE0); //has to be 0 or 1 or ...
        withProgram->setUniformValue(arrayIndexVariableName,(float)tex->getIndex()); //index for the texture array
    }


}

