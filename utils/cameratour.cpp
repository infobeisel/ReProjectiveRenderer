#include "utils/cameratour.h"
#include <QVector3D>
#include "assimpscenesearch.h"
#include "utils/CatmullRomSpline.h"
#include <QDebug>
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
    if(s->HasCameras() && s->mNumCameras > 4) {
        for(int i = 0; i < s->mNumCameras; i++) {
            aiCamera* li = s->mCameras[i];
            aiNode* found = AssimSceneSearch::find(QString(li->mName.data),s->mRootNode);
            aiVector3t<float> tPos = aiVector3t<float>(0.0f,0.0f,0.0f);
            aiVector3t<float> tScl = aiVector3t<float>(0.0f,0.0f,0.0f);
            aiQuaterniont<float> q = aiQuaterniont<float>();
            if(found != NULL) {
                //Decompose (aiVector3t<TReal>& pScaling, aiQuaterniont<TReal>& pRotation,
                //    aiVector3t<TReal>& pPosition) const
                found->mTransformation.Decompose(tScl,q,tPos);
            }
            cameraPosition = QVector3D(li->mPosition[0] + tPos[0],li->mPosition[1] + tPos[1],li->mPosition[2] + tPos[2]);
            aiVector3D tl = li->mLookAt;//q.Rotate(li->mLookAt);
            cameraForward = QVector3D(tl[0],tl[1],tl[2]).normalized();
            tl = li->mUp;//q.Rotate(li->mUp);
            cameraUp =QVector3D(tl[0],tl[1],tl[2]).normalized();

            tour->AddPosition(cameraPosition,timeStep * (float) i);
            forwards->AddPosition(cameraForward,timeStep * (float) i);
            ups->AddPosition(cameraUp,timeStep * (float) i);
        }
    } else {
        qWarning() << "CameraTour.cpp: given scene doesn't contain enough cameras: " <<  s->mNumCameras;
        return;
    }

    tour->FinalizeCreation();
    forwards->FinalizeCreation();
    ups->FinalizeCreation();
    valid = true;

}
bool CameraTour::isValid() {return valid;}
void CameraTour::getPositionForwardUp(float normalizedTime,QVector3D& position,QVector3D& forward, QVector3D& up) {
    position = tour->GetPosition(normalizedTime);
    forward = forwards->GetPosition(normalizedTime);
    up = ups->GetPosition(normalizedTime);
}

