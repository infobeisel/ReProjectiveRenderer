#include "scene.h"
#include <QVector>
#include <QImage>
Scene::Scene(QString fullPath,QOpenGLFunctions* tgl)
{
    gl = tgl;
    int i = fullPath.lastIndexOf("/");
    basePath = fullPath.left(i+1);
    name = fullPath.right(fullPath.size() -  i - 1);
}
Scene::~Scene()
{

    vertexArrayObject.destroy();
    vertexBufferObject.destroy();
    normalBufferObject.destroy();
    textureUVBufferObject.destroy();
    indexBufferObject.destroy();
    gl->glDeleteTextures(1,&textureArray);
    s->~aiScene();
}


/**
 * @brief reads the file which was specified by fullPath param in the constructor and generates and fills opengl buffer objects
 * NOTE: assumes 1 uv channel for each vertex, ignores the rest (during rendering the same uv coord is used for every texture lookup, if a mesh has multiple textures)
 */
void Scene::load() {
    QString path = basePath + name;
    s = importer.ReadFile( path.toStdString(),aiProcessPreset_TargetRealtime_Quality);

    //create the data for the buffers

    QVector<float> vertices = QVector<float>(0);
    QVector<float> normals = QVector<float>(0);
    QVector<float> textureUVs = QVector<float>(0); //Caution: assuming 1 uv channel.
    QVector<unsigned int> indices  = QVector<unsigned int>(0);

    //go through all nodes
    //array to traverse the node tree
    QVector<aiNode*> nodes;
    nodes.push_back(s->mRootNode);

    while(nodes.size() != 0) {
        //handle current node
        aiNode* node = nodes.takeFirst();


        for(uint i = 0; i<node->mNumMeshes; i++) { //go through this node's meshes
            aiMesh* mesh = s->mMeshes[node->mMeshes[i]]; //the mesh taken from the scene meshes array with the indices provided in the node
            int offset = vertices.size()/3; // the offset which should be added on the indices (to let them point on the correct place in the vertex array)

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

                    if(mesh->HasTextureCoords(0)) {
                        //assuming only one uv channel (one set of uv coordinates)
                        aiVector3D &uv = mesh->mTextureCoords[0][j];
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
                //TODO ignore tangent and bitangent vectors for now


            }
        }
        //add all children
        for(uint k = 0; k < node->mNumChildren; k++)
            nodes.push_back(node->mChildren[k]);
    }





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


    //now load the textures. Material parameters can be read directly while drawing when traversing the node tree
    //go through the materials to gather all textures in an array. also store the index
    QVector<QImage*> textures;
    int index = 0;
    //hold the max width and height of textures for allocation. will result in unused allocated memory if different resolutions are used, but
    //it's state of the art to use unified texture resolutions anyway
    int maxwidth = 0;
    int maxheight = 0;

    for(uint i = 0; i < s->mNumMaterials; i++) {
        aiMaterial* mat = s->mMaterials[i];
        //support ambient, diffuse and specular texture, as well as dissolve mask and normal map. TODO dissolve map type could not be found
        QVector<aiTextureType> types;
        types.push_back(aiTextureType_DIFFUSE);
        //types.push_back(aiTextureType_SPECULAR);
        //types.push_back(aiTextureType_AMBIENT);
        //types.push_back(aiTextureType_HEIGHT); //the bump maps? alternatively aiTextureType_NORMALS

        for(int j = 0; j < types.size(); j++) {
            aiString texPath;
            if (mat->GetTexture(types[j], 0, &texPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                QString texturePath = texPath.data;
                QImage* img = new QImage(basePath +  texturePath);
                QImage* mirrored = new QImage(img->width(),img->height(),img->format());
                (*mirrored) = img->mirrored();

                //if not already allocated
                if(texturePathToArrayIndex.find(texturePath) == texturePathToArrayIndex.end())
                {
                    textures.push_back(mirrored); //add the image to image array
                    texturePathToArrayIndex[texturePath] = index; //save index
                    int t_w = mirrored->width();
                    int t_h =   mirrored->height();
                    maxwidth = maxwidth < t_w ? t_w : maxwidth;
                    maxheight = maxheight < t_h ? t_h : maxheight;
                    index++;
                }
            }
        }
    }

    //now all available textures can be accessed in the textures array in format QImage. now set up the opengl server-side texture array
    //the mapping from material name to index in opengl texture array is stored in the texturePathToArrayIndex
    qDebug() << "creating OpenGl texture array with " << textures.size() << " textures of size " << maxwidth << "x" << maxheight;
    gl->glGenTextures(1,&textureArray);
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D_ARRAY,textureArray);
    glTexImage3D (GL_TEXTURE_2D_ARRAY, 0,  GL_RGBA , maxwidth, maxheight,
        textures.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    texRes = new int[2];
    texRes[0] = maxwidth;
    texRes[1] = maxheight;
    for(int imgi = 0; imgi<textures.size(); imgi++) { //add each texture to the texture array
        GLenum t_target = GL_TEXTURE_2D_ARRAY;
        //https://www.opengl->org/sdk/docs/man2/xhtml/glTexSubImage3D.xml
        /*
         * insert the layer in zoffset (5th param) and not in depth (8th param, insert 1 there).
         * zoffset means layer, depth (probably) means mip map level(?)
         * */
        glTexSubImage3D(t_target,
                        0,
                        0,0,imgi,
                        textures[imgi]->width(),textures[imgi]->height(),1,
                        GL_RGBA,         // format
                        GL_UNSIGNED_BYTE, // type
                        textures[imgi]->bits());
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    }
    gl->glBindTexture(GL_TEXTURE_2D_ARRAY, 0); //unbind


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
    indexBufferObject.bind();
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D_ARRAY,textureArray);
    toProgram->setUniformValue( "tex", 0 );

    vertexArrayObject.release();

}

void Scene::draw(QOpenGLShaderProgram *withProgram, QMatrix4x4 viewMatrix, QMatrix4x4 projMatrix){


    vertexArrayObject.bind();

    //go through all nodes (EXACTLY AS THEY WERE LOADED INTO THE VERTEX BUFFER)
    //array to traverse the node tree
    QVector<aiNode*> nodes;
    nodes.push_back(s->mRootNode);
    int indexOffsetCounter = 0;

    while(nodes.size() != 0) {
        //handle current node
        aiNode* node = nodes.takeFirst();

        QMatrix4x4 modelMatrix =  QMatrix4x4(node->mTransformation[0]);
        QMatrix4x4 modelViewMatrix = viewMatrix * modelMatrix;
        QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
        QMatrix4x4 mvp = projMatrix * modelViewMatrix;


        withProgram->setUniformValue( "MV", modelViewMatrix );
        withProgram->setUniformValue( "N", normalMatrix );
        withProgram->setUniformValue( "MVP", mvp );

        for(uint i = 0; i<(node->mNumMeshes); i++) { //go through this node's meshes
            int j = node->mMeshes[i];
            aiMesh** meshes = s->mMeshes;
            aiMesh* mesh = meshes[j]; //the mesh taken from the scene meshes array with the indices provided in the node
            //set all the mesh-specific parameters
            //materialparameters
            //Ka,Kd,Ks
            aiVector3D ka (0.f,0.f,0.f);
            aiVector3D kd (0.f,0.f,0.f);
            aiVector3D ks (0.f,0.f,0.f);
            aiMaterial* mat = s->mMaterials[mesh->mMaterialIndex];
            mat->Get( AI_MATKEY_COLOR_AMBIENT, ka);
            mat->Get( AI_MATKEY_COLOR_DIFFUSE, kd);
            mat->Get( AI_MATKEY_COLOR_SPECULAR, ks);
            withProgram->setUniformValue("Ka",QVector3D(ka.x,ka.y,ka.z));
            withProgram->setUniformValue("Kd",QVector3D(kd.x,kd.y,kd.z));
            withProgram->setUniformValue("Ks",QVector3D(ks.x,ks.y,ks.z));
            //assume triangles
            int count = mesh->mNumFaces * 3; //faces are triangles (assumption)
            GLenum err = gl->glGetError();
            while ( err != GL_NO_ERROR) {
                qDebug() << "gl error " << err;
                err = gl->glGetError();
            }


           /* GLint currentlybound;
            gl->glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&currentlybound );

            gl->glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&currentlybound );
*/


            gl->glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_INT,(const void*)(indexOffsetCounter * sizeof(unsigned int)) );

            indexOffsetCounter += count;

        }
        //add all children
        for(uint k = 0; k < node->mNumChildren; k++)
            nodes.push_back(node->mChildren[k]);
    }
    vertexArrayObject.release();

}
