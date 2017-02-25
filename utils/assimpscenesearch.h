#ifndef ASSIMPSCENESEARCH_H
#define ASSIMPSCENESEARCH_H

#include <assimp/scene.h>
#include <QString>
#include <QVector>

class AssimpSceneSearch
{
public:
    static aiNode* find(QString name,aiNode* root) {

        QVector<aiNode*> nodes;
        nodes.push_back(root);
        while(nodes.size() != 0) {
            aiNode* node = nodes.takeFirst();
            if( QString(node->mName.data) == name) {
                return node;
            }
            //add all children
            for(uint k = 0; k < node->mNumChildren; k++)
                nodes.push_back(node->mChildren[k]);
        }
        return nullptr;
    }
};





#endif // ASSIMPSCENESEARCH_H

