#ifndef PIXELCOUNTER_H
#define PIXELCOUNTER_H
#include "openglfunctions.h"
#include <QColor>
#include <QImage>
class PixelCounter
{
public:
    PixelCounter();
    void readFromGLTexture(GLuint texture);
    int countPixelsWithColor(QColor c); //counts absolute pixels with given color in the loaded image
    float countPixelsWithColorFraction(QColor c); // counts fraction of pixels which have given color, compared to the total pixel count
private:
    QImage image;

};

#endif // PIXELCOUNTER_H
