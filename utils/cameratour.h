#ifndef CAMERATOUR_H
#define CAMERATOUR_H

#include <QString>
#include "Spline.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

/**
 * @brief class to import and play a  camera tour.
 * Import file has to be an .fbx containing multiple cameras, ordered in the scene hierarchy as the desired sequence of control points
 * contained in the camera tour. control points are used to construct a spline, camera position and -orientation can be retrieved along the spline,
 * given the normalized time of the animation.
 *
 */
class CameraTour
{
public:
    CameraTour();
    CameraTour(QString pathToFile);
    ~CameraTour();
    void load(); // loads the fbx file containing the sequence of camera positons to generate the camera tour.
    void getPositionForwardUp(float normalizedTime,QVector3D& position,QVector3D& frwd, QVector3D& up);
    bool isValid();
    void setValid(bool v);
private:

    //the importer object and the scene from assimp
    const aiScene* s;
    Assimp::Importer importer;
    QString name; //name of the scene file
    QString basePath; //path to the scene file
    Framework::Math::Spline* tour; //positions
    Framework::Math::Spline* forwards;//forward vectors
    Framework::Math::Spline* ups;//up vectors
    bool valid;
};

#endif // CAMERATOUR_H
