#include "pixelcounter.h"
#include "openglfunctions.h"
#include <QtMath>
PixelCounter::PixelCounter()
{

}
void PixelCounter::saveReadImage(QString pathWithoutImageFormat){
    image.save(pathWithoutImageFormat,"BMP",100 );
}

float PixelCounter::calculateSSDToGLTexture(GLuint texture) {
    int w = -1;
    int h = -1;

    GL.glActiveTexture(GL_TEXTURE0);
    GL.glBindTexture(GL_TEXTURE_2D,texture);
    GL.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,&w);
    GL.glGetTexLevelParameteriv(GL_TEXTURE_2D,0, GL_TEXTURE_HEIGHT,&h);

    QImage image2;
    image2 = QImage( w,h,QImage::Format_RGBA8888 /*QImage::*/ );
    float ssd = 0;
    GL.glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,image2.bits());
    for(int i = 0; i < image2.width(); i++) {
        for(int j = 0; j < image2.height(); j++) {
            if(image.width() > i && image.height() > j) {

                QColor c1 = image.pixel(i,j);
                QColor c2 = image2.pixel(i,j);
                /* something mean squared
                float red_d = ((float)c1.red() - (float)c2.red() )*((float)c1.red() - (float)c2.red());
                float green_d = ((float)c1.green() - (float)c2.green() )*((float)c1.green() - (float)c2.green());
                float blue_d = ((float)c1.blue() - (float)c2.blue() )*((float)c1.blue() - (float)c2.blue());

                //float d = qSqrt(red_d+green_d+blue_d); //euclidian distance
                //squared distance:
                float d = red_d+green_d+blue_d;
                d /= (float)255.0f;
                ssd += d;*/

                //want mean absolute error

                float red_d = qAbs(((float)c1.red() - (float)c2.red() ));
                float green_d =  qAbs((float)c1.green() - (float)c2.green() );
                float blue_d =  qAbs((float)c1.blue() - (float)c2.blue() );

                float d = (red_d+green_d+blue_d) / 255.0f / 3.0f;

                ssd += d;




            }
        }
    }
    return ssd / image.width() / image.height();

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
