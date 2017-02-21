#ifndef CAMERATOUR_H
#define CAMERATOUR_H

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
private:

    //the importer object and the scene from assimp
    const aiScene* s;
    Assimp::Importer importer;
};

#endif // CAMERATOUR_H
