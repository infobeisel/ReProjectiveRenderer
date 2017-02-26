#include "pixelcounter.h"
#include "openglfunctions.h"
PixelCounter::PixelCounter()
{

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
