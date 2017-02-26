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
    void subtractGLTexture(GLuint texture);
    int countPixelsWithColor(QColor c); //counts absolute pixels with given color in the loaded image
    float countPixelsWithColorFraction(QColor c); // counts fraction of pixels which have given color, compared to the total pixel count
    void saveReadImage(QString pathWithoutImageFormat); //save the last read image
private:
    QImage image;

};

#endif // PIXELCOUNTER_H
