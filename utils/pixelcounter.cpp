#include "pixelcounter.h"
#include "openglfunctions.h"
#include <QtMath>
PixelCounter::PixelCounter()
{

}
void PixelCounter::saveReadImage(QString pathWithoutImageFormat){
    image.save(pathWithoutImageFormat,"BMP",100 );
}


void PixelCounter::subtractGLTexture(GLuint texture) {
    int w = -1;
    int h = -1;

    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,texture);
    GL.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,&w);
    GL.glGetTexLevelParameteriv(GL_TEXTURE_2D,0, GL_TEXTURE_HEIGHT,&h);

    QImage image2;
    image2 = QImage( w,h,QImage::Format_RGBA8888 /*QImage::*/ );

    GL.glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,image2.bits());
    for(int i = 0; i < image2.width(); i++) {
        for(int j = 0; j < image2.height(); j++) {
            if(image.width() > i && image.height() > j) {
                QColor c1 = image.pixel(i,j);
                QColor c2 = image2.pixel(i,j);
                image.setPixelColor(i,j,QColor(qFabs(c1.red() - c2.red()),qFabs(c1.blue() - c2.blue()),qFabs(c1.green() - c2.green()),qFabs(c1.alpha() - c2.alpha())));
            }
        }
    }



}

void PixelCounter::readFromGLTexture(GLuint texture) {

    int w = -1;
    int h = -1;

    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,texture);
    GL.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,&w);
    GL.glGetTexLevelParameteriv(GL_TEXTURE_2D,0, GL_TEXTURE_HEIGHT,&h);


    if (image.isNull()) //allocate
        image = QImage( w,h,QImage::Format_RGBA8888 /*QImage::*/ );

    GL.glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,image.bits());
    //image.save("path.bmp","BMP",100 );
}

int PixelCounter::countPixelsWithColor(QColor c) {
    int counter = 0;
    for(int i = 0; i < image.width(); i++) {
        for(int j = 0; j < image.height(); j++) {
            if(image.pixelColor(i,j) ==  c)
                counter++;
        }
    }
    return counter;

}
float PixelCounter::countPixelsWithColorFraction(QColor c) {
    int counter = countPixelsWithColor(c);
    return (float) counter / ((float)image.width() * (float)image.height());
}
