

#include "model.h"
#include "applogger.h"

class myStream :
    public Assimp::LogStream
{
public:
    // Constructor
    myStream()
    {
        // empty
    }
    // Destructor
    ~myStream()
    {
        // empty
    }
    // Write womethink using your own functionality
    void write(const char* message)
    {
        QString alteredMessage(message);
        alteredMessage.remove('\n');
        Logger::LogLevel currentLogLevel = GlobalLog.GetModuleLogLevel(LOG_OBJLOADER);
        GlobalLog.LogMessage(LOG_OBJLOADER, currentLogLevel, alteredMessage.toUtf8().constData());
    }
};

Model::Model()
{
    m_scene = NULL;

    Logger::LogLevel currentLogLevel = GlobalLog.GetModuleLogLevel(LOG_OBJLOADER);
    int assimpLogSeverity = 0;
    switch(currentLogLevel)
    {
    // deliberate fall through:
    case Logger::Debug2:
        assimpLogSeverity |= Assimp::Logger::DEBUGGING;
    case Logger::Debug1:
    case Logger::Info:
        assimpLogSeverity |= Assimp::Logger::INFO;
    case Logger::Warning:
        assimpLogSeverity |= Assimp::Logger::WARN;
    case Logger::Error:
        assimpLogSeverity |= Assimp::Logger::ERR;
        break;
    default:
        break;
    }

    // Create a logger instance
    Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);

    // Attach our custom stream to the default logger
    Assimp::DefaultLogger::get()->attachStream( new myStream(), assimpLogSeverity );

    Assimp::DefaultLogger::get()->info("this is my info-call");
}

Model::~Model()
{
    m_nodes.resize(0);

    if(m_scene)
    {
        aiReleaseImport(m_scene);
        m_scene = NULL;
    }

    aiDetachAllLogStreams();
}

void Model::aiNodesToVertexArrays()
{
    /* Depth first traverse node tree and place m_nodes in flat QList,
       then work on each node in QList to create usable arrays.

       Each node in tree has m_meshes, each mesh has faces, each face has indices
       one indice is set of co-ordinates/tex co-ords/colour/normal for one point (in a polygon say)

       Transformation is per node,
       Texture/material is per mesh, so we want 1 set of arrays for each mesh

       Get all the points out and put them in:
       QVector of m_nodes
           Transformation matrix ptr
           QVector of m_meshes
               texid
               vertices: QVector of QVector3D
               tex co-ords: QVector of QVector3D
               normals: QVector of QVector3D

       Only bother with triangles and see how that turns out.

    */

    QList<struct aiNode*> flatNodePtrList;
    struct aiNode* currentNode = m_scene->mRootNode;
    flatNodePtrList.prepend(currentNode);

    while(flatNodePtrList.size())
    {
        // Store children m_nodes to process next, removing the
        // current (parent) node from the front of the list:
        currentNode = flatNodePtrList.takeFirst();
        for(int childNodeIx = currentNode->mNumChildren-1; childNodeIx >= 0; --childNodeIx)
        {
            flatNodePtrList.prepend(currentNode->mChildren[childNodeIx]);
        }

        // Process the current node:
        ModelNode newModelNode;

        newModelNode.m_transformMatrix = QMatrix4x4((qreal)currentNode->mTransformation.a1,
                                                  (qreal)currentNode->mTransformation.a2,
                                                  (qreal)currentNode->mTransformation.a3,
                                                  (qreal)currentNode->mTransformation.a4,
                                                  (qreal)currentNode->mTransformation.b1,
                                                  (qreal)currentNode->mTransformation.b2,
                                                  (qreal)currentNode->mTransformation.b3,
                                                  (qreal)currentNode->mTransformation.b4,
                                                  (qreal)currentNode->mTransformation.c1,
                                                  (qreal)currentNode->mTransformation.c2,
                                                  (qreal)currentNode->mTransformation.c3,
                                                  (qreal)currentNode->mTransformation.c4,
                                                  (qreal)currentNode->mTransformation.d1,
                                                  (qreal)currentNode->mTransformation.d2,
                                                  (qreal)currentNode->mTransformation.d3,
                                                  (qreal)currentNode->mTransformation.d4);


        for(unsigned int meshIx = 0; meshIx < currentNode->mNumMeshes; ++meshIx)
        {
            const struct aiMesh* currentMesh = m_scene->mMeshes[currentNode->mMeshes[meshIx]];

            ModelMesh newModelMesh;

            // TODO: Grab texture info/load image file here....

            newModelMesh.m_hasNormals = currentMesh->HasNormals();
            newModelMesh.m_hasTexcoords = currentMesh->HasTextureCoords(0);

            for(unsigned int faceIx = 0; faceIx < currentMesh->mNumFaces; ++faceIx)
            {
                const struct aiFace* currentFace = &currentMesh->mFaces[faceIx];

                if(currentFace->mNumIndices != 3)
                {
                   LOG(LOG_OBJLOADER, Logger::Info, "Ignoring non-triangle mesh %d face %d\n", meshIx, faceIx);
                }


                for(unsigned int i = 0; i < currentFace->mNumIndices; i++)
                {
                    int vertexIndex = currentFace->mIndices[i];

                    QVector3D vert(currentMesh->mVertices[vertexIndex].x, currentMesh->mVertices[vertexIndex].y, currentMesh->mVertices[vertexIndex].z);
                    newModelMesh.m_triangleVertices.append(vert);

                    if(newModelMesh.m_hasNormals)
                    {
                        QVector3D norm(currentMesh->mNormals[vertexIndex].x, currentMesh->mNormals[vertexIndex].y, currentMesh->mNormals[vertexIndex].z);
                        newModelMesh.m_triangleNormals.append(norm);
                    }

                    if(newModelMesh.m_hasTexcoords)
                    {
                        QVector2D tex(currentMesh->mTextureCoords[0][vertexIndex].x, 1 - currentMesh->mTextureCoords[0][vertexIndex].y);
                        newModelMesh.m_triangleTexcoords.append(tex);
                    }

                }
            }

            newModelNode.m_meshes.append(newModelMesh);
        }

        m_nodes.append(newModelNode);
    }
}


int Model::Load(QString fileName)
{
    if(m_scene)
    {
        // Clear extracted node data
        m_nodes.resize(0);

        aiReleaseImport(m_scene);
        m_scene = NULL;
    }

    // Load model
    m_scene = aiImportFile(fileName.toUtf8().constData(), aiProcessPreset_TargetRealtime_Quality);

    if (!m_scene)
    {
        LOG(LOG_OBJLOADER, Logger::Error, "Couldn't load obj model file %s", fileName.toUtf8().constData());
        return -1;
    }

    // Extract from ai mesh/faces into arrays
    aiNodesToVertexArrays();

    // Get the offset to center the model about the origin when drawing later
    get_bounding_box(&m_sceneMin,&m_sceneMax);
    m_sceneCenter.x = (m_sceneMin.x + m_sceneMax.x) / 2.0f;
    m_sceneCenter.y = (m_sceneMin.y + m_sceneMax.y) / 2.0f;
    m_sceneCenter.z = (m_sceneMin.z + m_sceneMax.z) / 2.0f;

    // Sensible default
    m_scaleFactor = 1.0;

    return 0;
}

void Model::SetScale(qreal boundarySize)
{
    if (!m_scene)
    {
        LOG(LOG_OBJLOADER, Logger::Warning, "Model file not loaded yet");
        return;
    }

    float longestSide = m_sceneMax.x-m_sceneMin.x;
    longestSide = qMax(m_sceneMax.y - m_sceneMin.y, longestSide);
    longestSide = qMax(m_sceneMax.z - m_sceneMin.z, longestSide);

    m_scaleFactor = boundarySize / (qreal)longestSide;
}

void Model::Draw(QMatrix4x4 modelViewMatrix, QMatrix4x4 projectionMatrix, QGLShaderProgram *shaderProg, bool useModelTextures)
{
    if (!m_scene)
    {
        LOG(LOG_OBJLOADER, Logger::Warning, "Model file not loaded yet");
        return;
    }

    // Center and scale the model
    modelViewMatrix.scale(m_scaleFactor);
    modelViewMatrix.translate(-m_sceneCenter.x, -m_sceneCenter.y, -m_sceneCenter.z);

    foreach(ModelNode node, m_nodes)
    {
        QMatrix4x4 nodeModelViewMatrix = modelViewMatrix * node.m_transformMatrix;

        // Load modelview projection matrix into shader. The projection matrix must
        // be multiplied by the modelview, not the other way round!
        shaderProg->setUniformValue("u_mvp_matrix", projectionMatrix * nodeModelViewMatrix);
        shaderProg->setUniformValue("u_mv_matrix", nodeModelViewMatrix);

        foreach(ModelMesh mesh, node.m_meshes)
        {
            if(useModelTextures)
            {
                // Set/enable texture id if desired ....
            }

            if(mesh.m_hasNormals)
            {
                shaderProg->enableAttributeArray("a_normal");
                shaderProg->setAttributeArray("a_normal", mesh.m_triangleNormals.constData());
            }

            if(mesh.m_hasTexcoords)
            {
                shaderProg->enableAttributeArray("a_texCoord");
                shaderProg->setAttributeArray("a_texCoord", mesh.m_triangleTexcoords.constData());
            }

            shaderProg->enableAttributeArray("a_vertex");
            shaderProg->setAttributeArray("a_vertex", mesh.m_triangleVertices.constData());

            glDrawArrays(GL_TRIANGLES, 0, mesh.m_triangleVertices.size());
            shaderProg->disableAttributeArray("a_vertex");
            shaderProg->disableAttributeArray("a_normal");
            shaderProg->disableAttributeArray("a_texCoord");
        }
    }
}


void Model::get_bounding_box_for_node (const struct aiNode* nd,
        struct aiVector3D* min,
        struct aiVector3D* max,
        struct aiMatrix4x4* trafo)
{
        struct aiMatrix4x4 prev;
        unsigned int n = 0, t;

        prev = *trafo;
        aiMultiplyMatrix4(trafo,&nd->mTransformation);

        for (; n < nd->mNumMeshes; ++n) {
                const struct aiMesh* mesh = m_scene->mMeshes[nd->mMeshes[n]];
                for (t = 0; t < mesh->mNumVertices; ++t) {

                        struct aiVector3D tmp = mesh->mVertices[t];
                        aiTransformVecByMatrix4(&tmp,trafo);

                        min->x = qMin(min->x,tmp.x);
                        min->y = qMin(min->y,tmp.y);
                        min->z = qMin(min->z,tmp.z);

                        max->x = qMax(max->x,tmp.x);
                        max->y = qMax(max->y,tmp.y);
                        max->z = qMax(max->z,tmp.z);
                }
        }

        for (n = 0; n < nd->mNumChildren; ++n) {
                get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
        }
        *trafo = prev;
}

void Model::get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
        struct aiMatrix4x4 trafo;
        aiIdentityMatrix4(&trafo);

        min->x = min->y = min->z =  1e10f;
        max->x = max->y = max->z = -1e10f;
        get_bounding_box_for_node(m_scene->mRootNode,min,max,&trafo);
}

