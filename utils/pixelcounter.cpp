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
    image.save("path.png");
}

int PixelCounter::countPixelsWithColor(QColor c) {


}
float PixelCounter::countPixelsWithColorFraction(QColor c) {

}
