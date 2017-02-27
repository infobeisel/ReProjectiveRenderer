#include "utils/cameratour.h"
#include <QVector3D>
#include "assimpscenesearch.h"
#include "utils/CatmullRomSpline.h"
#include <QDebug>
#include <QMatrix4x4>
CameraTour::CameraTour(QString fullPath)
{
    int i = fullPath.lastIndexOf("/");
    basePath = fullPath.left(i+1);
    name = fullPath.right(fullPath.size() -  i - 1);
    valid = false;
}
CameraTour::CameraTour()
{
    valid = false;
}
CameraTour::~CameraTour() {
    delete tour;
    delete forwards;
    delete ups;
}

void CameraTour::load()
{
    QString path = basePath + name;
    //importer and scene are both held as member to avoid scene deletion (due to importer deallocation)
    s = importer.ReadFile( path.toStdString(),aiProcessPreset_TargetRealtime_Quality);
    qDebug() << importer.GetErrorString();

    //feed positions into spline
    tour = new Framework::Math::CatmullRomSpline();
    forwards = new Framework::Math::CatmullRomSpline();
    ups = new Framework::Math::CatmullRomSpline();

    QVector3D cameraPosition;
    QVector3D cameraForward;
    QVector3D cameraUp;
    float timeStep = 1.0f / (float)(s->mNumCameras - 1);
    if(s->HasCameras() && s->mNumCameras >= 4) {
        for(int i = 0; i < s->mNumCameras; i++) {
            aiCamera* li = s->mCameras[i];
            aiNode* found = AssimpSceneSearch::find(QString(li->mName.data),s->mRootNode);
            QMatrix4x4 modelMatrix =  QMatrix4x4(found->mTransformation[0]);

            aiVector3t<float> tPos = aiVector3t<float>(0.0f,0.0f,0.0f);
            aiVector3t<float> tScl = aiVector3t<float>(0.0f,0.0f,0.0f);
            aiQuaterniont<float> q = aiQuaterniont<float>();
            if(found != NULL) {
                //Decompose (aiVector3t<TReal>& pScaling, aiQuaterniont<TReal>& pRotation,
                //    aiVector3t<TReal>& pPosition) const
                //found->mTransformation.Decompose(tScl,q,tPos);
            }
            cameraPosition = modelMatrix.map(QVector3D( tPos[0], tPos[1],tPos[2]));
            //aiVector3D tl = q.Rotate(li->mLookAt);//li->mLookAt;//
            //cameraForward = QVector3D(tl[0],tl[1],tl[2]).normalized();
            //cameraForward = modelMatrix.mapVector(QVector3D(0.0f,0.0f,1.0f));
            cameraForward = modelMatrix.mapVector(QVector3D(li->mLookAt[0],li->mLookAt[1],li->mLookAt[2]));
            cameraForward.normalize();
            //tl = q.Rotate(li->mUp);//li->mUp;//q.Rotate(li->mUp);
            //cameraUp =QVector3D(tl[0],tl[1],tl[2]).normalized();
            //cameraUp = modelMatrix.mapVector(QVector3D(0.0f,1.0f,0.0f));
            cameraUp = modelMatrix.mapVector(QVector3D(li->mUp[0],li->mUp[1],li->mUp[2]));
            cameraUp.normalize();
            tour->AddPosition(cameraPosition,timeStep * (float) i);
            forwards->AddPosition(-cameraForward,timeStep * (float) i);
            ups->AddPosition(cameraUp,timeStep * (float) i);
        }
    } else {
        qWarning() << "CameraTour.cpp: given scene doesn't contain enough cameras: " <<  s->mNumCameras;
        return;
    }

    tour->FinalizeCreation();
    forwards->FinalizeCreation();
    ups->FinalizeCreation();
    setValid(true);

}
bool CameraTour::isValid() {return valid;}
void CameraTour::setValid(bool v) {valid = v;}
void CameraTour::getPositionForwardUp(float normalizedTime,QVector3D& position,QVector3D& forward, QVector3D& up) {
    position = tour->GetPosition(normalizedTime);
    forward = forwards->GetPosition(normalizedTime);
    up = ups->GetPosition(normalizedTime);
}

