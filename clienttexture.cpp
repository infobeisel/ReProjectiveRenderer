#include "clienttexture.h"

ClientTexture::ClientTexture(QString tname, QImage* timg)
{
    name = tname;
    img = timg;
}

QImage* ClientTexture::image() {
    return img;
}

int ClientTexture::width() {
    return img->width();
}

int ClientTexture::height() {
    return img->height();
}

QString ClientTexture::getName(){
    return name;
}

int ClientTexture::getIndex() {
    return index;
}

void ClientTexture::setIndex(int tindex) {
    index = tindex;
}

