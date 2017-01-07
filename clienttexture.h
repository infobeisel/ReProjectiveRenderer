#ifndef CLIENTTEXTURE_H
#define CLIENTTEXTURE_H
#include <QString>
#include <QImage>


class ClientTexture
{
public:
    ClientTexture(QString tname, QImage* timg);
    QImage* image();
    int width();
    int height();
    QString getName();
    int getIndex(); //returns the index where it is located in the texture array
    void setIndex(int tindex);
private:
    int index;
    QImage* img;
    QString name;


};

#endif // CLIENTTEXTURE_H
