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
}

void CameraTour::load()
{
    QString path = basePath + name;
    //importer and scene are both held as member to avoid scene deletion (due to importer deallocation)
    s = importer.ReadFile( path.toStdString(),aiProcessPreset_TargetRealtime_Quality);
    qDebug() << importer.GetErrorString();

    //feed positions into spline
    tour = new Framework::Math::CatmullRomSpline();

    QVector3D cameraPosition;
    float timeStep = 1.0f / (float)s->mNumCameras;
    if(s->HasCameras() && s->mNumCameras > 4) {
        for(int i = 0; i < s->mNumCameras; i++) {
            aiCamera* li = s->mCameras[i];
            aiNode* found = AssimSceneSearch::find(QString(li->mName.data),s->mRootNode);
            aiVector3D tPos = aiVector3D(0.0f,0.0f,0.0f);
            if(found != NULL) {
                tPos = aiVector3D(found->mTransformation.a4,found->mTransformation.b4,found->mTransformation.c4);
            }
            cameraPosition = QVector3D(li->mPosition[0] + tPos[0],li->mPosition[1] + tPos[1],li->mPosition[2] + tPos[2]);
            tour->AddPosition(cameraPosition,timeStep * (float) i);

        }
    } else {
        qWarning() << "CameraTour.cpp: given scene doesn't contain enough cameras: " <<  s->mNumCameras;
        return;
    }

    tour->SetStartTimeToZero();
    tour->FinalizeCreation();
    valid = true;

}
bool CameraTour::isValid() {return valid;}
void CameraTour::getPositionTangentNormal(float normalizedTime,QVector3D& position,QVector3D& tangent, QVector3D& normal) {
    position = tour->GetPosition(normalizedTime);
    tour->GetTangentAndNormal(normalizedTime,tangent,normal);
}

