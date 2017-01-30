#include "scene.h"
#include <QVector>
#include <QImage>



Scene::Scene(QString fullPath,QOpenGLFunctions_4_1_Core* tgl)
{
    gl = tgl;
    int i = fullPath.lastIndexOf("/");
    basePath = fullPath.left(i+1);
    name = fullPath.right(fullPath.size() -  i - 1);
}

Scene::~Scene()
{

    delete s;
    delete textureManager;
}

/**
 * @brief reads the file which was specified by fullPath param in the constructor and generates and fills opengl buffer objects
 * NOTE: assumes 1 uv channel for each vertex, ignores the rest (during rendering the same uv coord is used for every texture lookup, if a mesh has multiple textures)
 */
void Scene::load(QOpenGLShaderProgram *toProgram) {
    QString path = basePath + name;
    //importer and scene are both held as member to avoid scene deletion (due to importer deallocation)
    s = importer.ReadFile( path.toStdString(),aiProcessPreset_TargetRealtime_Quality);
    qDebug() << importer.GetErrorString();
    //initi texture manager
    textureManager = new ClientTextureArrayManager();
    //create the data for the buffers
    QVector<float> vertices = QVector<float>(0);
    QVector<float> normals = QVector<float>(0);
    QVector<float> tangents = QVector<float>(0);
    QVector<float> textureUVs = QVector<float>(0); //Caution: assuming 1 uv channel.
    QVector<unsigned int> indices  = QVector<unsigned int>(0);

    //load the lights. store them in a map which maps node name to aiLight datastructure
    //lights: read lightsources array
    QMap<QString,int> lightSourceNameToLightArrayIndex;
    if(s->HasLights()) {
        for(int i = 0; i < s->mNumLights; i++) {
            aiLight* li = s->mLights[i];
            QString lightName = li->mName.data;
            //qDebug() << "found light in scene " << li->mName.data << " " << li->mType ;
            lightSourceNameToLightArrayIndex[lightName] = i;
        }
    }

    //go through all nodes
    //array to traverse the node tree
    QVector<aiNode*> nodes;
    nodes.push_back(s->mRootNode);
    while(nodes.size() != 0) {
        //handle current node
        aiNode* node = nodes.takeFirst();
        //is this node a light source? if yes update position and direction into the aiLight datastructure
        if(s->HasLights()) {
            if(lightSourceNameToLightArrayIndex.find(QString(node->mName.data)) != lightSourceNameToLightArrayIndex.end()) {
                QMatrix4x4 modelMatrix =  QMatrix4x4(node->mTransformation[0]);
                QVector3D dir = modelMatrix.map(QVector3D(0.0f,0.0f,1.0f));
                //qDebug() << QString(node->mName.data) << QVector4D(node->mTransformation.a4,node->mTransformation.b4,node->mTransformation.c4,node->mTransformation.d4);
                int lightSourceIndex = lightSourceNameToLightArrayIndex[QString(node->mName.data)];
                aiLight* li = s->mLights[lightSourceIndex];
                //update positions! (assimp doesnt store light position and direction into the aiLight datastructure, only into the node's transformation
                li->mPosition = aiVector3D(node->mTransformation.a4,node->mTransformation.b4,node->mTransformation.c4);
                li->mDirection = aiVector3D(dir.x(),dir.y(),dir.z());
            }
        }
        for(uint i = 0; i<node->mNumMeshes; i++) { //go through this node's meshes
            //the mesh taken from the scene meshes array with the indices provided in the node
            aiMesh* mesh = s->mMeshes[node->mMeshes[i]];
            // the offset which should be added on the indices
            // (to let them point on the correct place in the vertex array)
            int offset = vertices.size()/3;
            if(mesh->mNumVertices > 0) {//add vertices, indices and normals
                for(uint j=0; j<mesh->mNumVertices; j++) {
                    aiVector3D &vec = mesh->mVertices[j];
                    vertices.push_back(vec.x);
                    vertices.push_back(vec.y);
                    vertices.push_back(vec.z);
                    aiVector3D &norm = mesh->mNormals[j];
                    normals.push_back(norm.x);
                    normals.push_back(norm.y);
                    normals.push_back(norm.z);
                    if(mesh->HasTangentsAndBitangents()) {
                        aiVector3D &tangent = mesh->mTangents[j];
                        tangents.push_back(tangent.x);
                        tangents.push_back(tangent.y);
                        tangents.push_back(tangent.z);
                    } else {
                        qDebug() << "WARNING: " << mesh->mName.data <<" Tangents set to (0,0,0)";
                        tangents.push_back(0.0f);
                        tangents.push_back(0.0f);
                        tangents.push_back(0.0f);
                    }

                    if(mesh->HasTextureCoords(0)) {
                        //assuming only one uv channel (one set of uv coordinates)
                        aiVector3D &uv = mesh->mTextureCoords[0][j];
                        //qDebug() << uv.x << " " << uv.y;
                        textureUVs.push_back(uv.x);
                        textureUVs.push_back(uv.y);
                    } else {
                        textureUVs.push_back(0.0f);
                        textureUVs.push_back(0.0f);
                    }
                }
                for(uint j = 0; j < mesh->mNumFaces; j++) {
                    aiFace face = mesh->mFaces[j];
                    //assuming triangles
                    indices.push_back(face.mIndices[0] + offset);
                    indices.push_back(face.mIndices[1] + offset);
                    indices.push_back(face.mIndices[2] + offset);
                }

            }
        }
        //add all children
        for(uint k = 0; k < node->mNumChildren; k++)
            nodes.push_back(node->mChildren[k]);
    }
    //create and fill the opengl server-side buffers
    if(!vertexArrayObject.create())
        qDebug() << "ERROR creating vertex Array Object" ;
    vertexArrayObject.bind();
    vertexBufferObject = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexBufferObject.create();
    vertexBufferObject.setUsagePattern( QOpenGLBuffer::StaticDraw );
    vertexBufferObject.bind();
    vertexBufferObject.allocate( &vertices[0], vertices.size() * sizeof( float ) );
    normalBufferObject = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    normalBufferObject.create();
    normalBufferObject.setUsagePattern( QOpenGLBuffer::StaticDraw );
    normalBufferObject.bind();
    normalBufferObject.allocate( &normals[0], normals.size() * sizeof( float ) );


    tangentBufferObject = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    tangentBufferObject.create();
    tangentBufferObject.setUsagePattern( QOpenGLBuffer::StaticDraw );
    tangentBufferObject.bind();
    tangentBufferObject.allocate( &tangents[0], tangents.size() * sizeof( float ) );

    if(textureUVs.size() != 0)
    {
        textureUVBufferObject.create();
        textureUVBufferObject.setUsagePattern( QOpenGLBuffer::StaticDraw );
        textureUVBufferObject.bind();
        textureUVBufferObject.allocate( &textureUVs[0], textureUVs.size() * sizeof( float ) );
    }
    indexBufferObject = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexBufferObject.create();
    indexBufferObject.setUsagePattern( QOpenGLBuffer::StaticDraw );
    indexBufferObject.bind();
    indexBufferObject.allocate( &indices[0], indices.size() * sizeof( unsigned int ) );

    //now load the images. Material parameters can be read directly while drawing when traversing the node tree
    for(uint i = 0; i < s->mNumMaterials; i++) {
        aiMaterial* mat = s->mMaterials[i];
        //support ambient, diffuse and specular texture, as well as dissolve mask and normal map. TODO dissolve map type could not be found
        QVector<aiTextureType> types;
        types.push_back(aiTextureType_DIFFUSE);
        types.push_back(aiTextureType_SPECULAR);
        types.push_back(aiTextureType_AMBIENT);
        types.push_back(aiTextureType_HEIGHT); //the bump maps? alternatively i'll try aiTextureType_NORMALS
        for(int j = 0; j < types.size(); j++) {
            aiString texPath;
            if (mat->GetTexture(types[j], 0, &texPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                QString texturePath = texPath.data;
                QImage* img = new QImage(basePath +  texturePath);
                QImage* mirrored = new QImage(img->width(),img->height(),img->format());
                (*mirrored) = img->mirrored();
                //add to manager
                textureManager->addImage(texturePath,mirrored);
            }
        }
    }

    textureManager->loadToServer(gl);




    vertexArrayObject.release();
}


void Scene::bind(QOpenGLShaderProgram *toProgram) {
    vertexArrayObject.bind();
    //vertex data to shaders layout location 0
    if(!vertexBufferObject.bind())
        qDebug() << "ERROR binding vbo";
    toProgram->enableAttributeArray(0);
    toProgram->setAttributeBuffer( 0,GL_FLOAT,0,3);
    //normal data to shaders layout location 1
    if(!normalBufferObject.bind())
        qDebug() << "ERROR binding nbo";
    toProgram->enableAttributeArray(1);
    toProgram->setAttributeBuffer( 1,GL_FLOAT,0,3);
    //uv data to shaders layout location 2
    if(textureUVBufferObject.isCreated()) {
        if(!textureUVBufferObject.bind())
            qDebug() << "ERROR binding textureuvbo";
        toProgram->enableAttributeArray(2);
        toProgram->setAttributeBuffer( 2,GL_FLOAT,0,2);
    }
    //tangent data to shaders layout location 3
    if(!tangentBufferObject.bind())
        qDebug() << "ERROR binding tangentBufferObject";
    toProgram->enableAttributeArray(3);
    toProgram->setAttributeBuffer( 3,GL_FLOAT,0,3);



    indexBufferObject.bind();

    vertexArrayObject.release();
}

void Scene::draw(QOpenGLShaderProgram *withProgram, QMatrix4x4 viewMatrix, QMatrix4x4 projMatrix){
    vertexArrayObject.bind();

    //tell the shader how many light sources there are
    withProgram->setUniformValue( "lightCount", s->mNumLights );
    //go through all lights and set the light uniforms
    if(s->HasLights()) {
        for(int i = 0; i < s->mNumLights; i++) {
            aiLight* li = s->mLights[i];
            //position and direction are stored in the accordin node!
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].position").toStdString().c_str(),
                                          (QVector3D(li->mPosition.x,li->mPosition.y,li->mPosition.z)) ); //world coordinates
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].direction").toStdString().c_str(),
                                          QVector3D(li->mDirection.x,li->mDirection.y,li->mDirection.z));
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].ambient").toStdString().c_str(),
                                          QVector3D(li->mColorAmbient.r,li->mColorAmbient.g,li->mColorAmbient.b) );
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].diffuse").toStdString().c_str(),
                                          QVector3D(li->mColorDiffuse.r,li->mColorDiffuse.g,li->mColorDiffuse.b) );
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].specular").toStdString().c_str(),
                                          QVector3D(li->mColorSpecular.r,li->mColorSpecular.g,li->mColorSpecular.b) );
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].attenuationConstant").toStdString().c_str(),
                                          (float)li->mAttenuationConstant );
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].attenuationLinear").toStdString().c_str(),
                                          (float)li->mAttenuationLinear );
            withProgram->setUniformValue( QString("lights["+QString::number(i)+"].attenuationQuadratic").toStdString().c_str(),
                                          (float)li->mAttenuationQuadratic );
        }
    }



    //go through all nodes (EXACTLY AS THEY WERE LOADED INTO THE VERTEX BUFFER)
    //array to traverse the node tree
    QVector<aiNode*> nodes;
    nodes.push_back(s->mRootNode);

    int indexOffsetCounter = 0;
    while(nodes.size() != 0) {
        //handle current node
        aiNode* node = nodes.takeFirst();
        //qDebug() << QString(node->mName.data) << node->mTransformation.a4 <<node->mTransformation.b4 <<node->mTransformation.c4 <<node->mTransformation.d4  ;
        QMatrix4x4 modelMatrix =  QMatrix4x4(node->mTransformation[0]);
        QMatrix4x4 modelViewMatrix = viewMatrix * modelMatrix;
        QMatrix3x3 n = modelMatrix.normalMatrix(); // lighting is calculated in world space
        QMatrix4x4 vp = projMatrix * viewMatrix;
        QMatrix4x4 mvp = vp * modelMatrix;
        withProgram->setUniformValue( "M", modelMatrix );
        withProgram->setUniformValue( "MV", modelViewMatrix );
        withProgram->setUniformValue( "MVP", mvp );
        withProgram->setUniformValue( "VP", vp );
        withProgram->setUniformValue( "NormalM", n );

        for(uint i = 0; i < node->mNumMeshes; i++) { //go through this node's meshes
            int j = node->mMeshes[i];
            aiMesh** meshes = s->mMeshes;
            aiMesh* mesh = meshes[j]; //the mesh taken from the scene meshes array with the indices provided in the node
            //set all the mesh-specific parameters
            //materialparameters
            //Ka,Kd,Ks
            aiColor3D ka (0.f,0.f,0.f);
            aiColor3D kd (0.f,0.f,0.f);
            aiColor3D ks (0.f,0.f,0.f);
            float shininess = 1.0f;
            aiMaterial* mat = s->mMaterials[mesh->mMaterialIndex];

            mat->Get( AI_MATKEY_COLOR_AMBIENT, ka);
            mat->Get( AI_MATKEY_COLOR_DIFFUSE, kd);
            mat->Get( AI_MATKEY_COLOR_SPECULAR, ks);
            mat->Get( AI_MATKEY_SHININESS, shininess );

            withProgram->setUniformValue("Ka",QVector3D(ka.r,ka.g,ka.b));
            withProgram->setUniformValue("Kd",QVector3D(kd.r,kd.g,kd.b));
            withProgram->setUniformValue("Ks",QVector3D(ks.r,ks.g,ks.b));
            withProgram->setUniformValue("specularExponent",shininess);

            //set texture parameters
            aiString tpath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &tpath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                textureManager->bindLoadedTexture(QString(tpath.data),&"diffuseSampler"[0],&"diffuseTextureArrayIndex"[0],withProgram);
            } else
                textureManager->bindLoadedTexture(QString(tpath.data),&"diffuseSampler"[0],&"diffuseTextureArrayIndex"[0],withProgram);

            if (mat->GetTexture(aiTextureType_AMBIENT, 0, &tpath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                textureManager->bindLoadedTexture(QString(tpath.data),&"ambientSampler"[0],&"ambientTextureArrayIndex"[0],withProgram);
            } else
                textureManager->bindLoadedTexture(QString(tpath.data),&"ambientSampler"[0],&"ambientTextureArrayIndex"[0],withProgram);

            if (mat->GetTexture(aiTextureType_SPECULAR, 0, &tpath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                textureManager->bindLoadedTexture(QString(tpath.data),&"specularSampler"[0],&"specularTextureArrayIndex"[0],withProgram);
            } else
                textureManager->bindLoadedTexture(QString(tpath.data),&"specularSampler"[0],&"specularTextureArrayIndex"[0],withProgram);

            if (mat->GetTexture(aiTextureType_HEIGHT, 0, &tpath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                textureManager->bindLoadedTexture(QString(tpath.data),&"bumpSampler"[0],&"bumpTextureArrayIndex"[0],withProgram);
            } else
                textureManager->bindLoadedTexture(QString(tpath.data),&"bumpSampler"[0],&"bumpTextureArrayIndex"[0],withProgram);


            //assume triangles
            int count = mesh->mNumFaces * 3; //faces are triangles (assumption)
            gl->glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_INT,(const void*)(indexOffsetCounter * sizeof(unsigned int)) );
            indexOffsetCounter += count; //move offset
        }
        //add all children
        for(uint k = 0; k < node->mNumChildren; k++)
            nodes.push_back(node->mChildren[k]);
    }
    vertexArrayObject.release();
}
