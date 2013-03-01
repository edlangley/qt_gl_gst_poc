#ifndef MODEL_H
#define MODEL_H

#include <QList>
#include <QVector2D>
#include <QVector3D>
#include <QGLShaderProgram>

#include "assimp.h"
#include "aiPostProcess.h"
#include "aiScene.h"
#include "DefaultLogger.h"
#include "LogStream.h"

class ModelMesh
{
public:
    QVector<QVector3D> m_triangleVertices;
    bool m_hasNormals;
    QVector<QVector3D> m_triangleNormals;
    bool m_hasTexcoords;
    QVector<QVector2D> m_triangleTexcoords;

    // Could add more QVectors here for points, lines, polys.

    // texture related members here ....

signals:

public slots:

};

class ModelNode
{
public:
    QVector<ModelMesh> m_meshes;
    QMatrix4x4 m_transformMatrix;
    //struct aiMatrix4x4 aim_transformMatrix;
signals:

public slots:

};

class Model
{
public:
    Model();
    ~Model();

    int Load(QString fileName);
    void SetScale(qreal boundarySize);
    void Draw(QMatrix4x4 modelViewMatrix, QMatrix4x4 projectionMatrix, QGLShaderProgram *shaderProg, bool useModelTextures);

private:
    void aiNodesToVertexArrays();
    void get_bounding_box_for_node (const struct aiNode* nd,
            struct aiVector3D* min,
            struct aiVector3D* max,
            struct aiMatrix4x4* trafo);
    void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);

    const struct aiScene* m_scene;
    QVector<ModelNode> m_nodes;
    struct aiVector3D m_sceneCenter;
    struct aiVector3D m_sceneMin;
    struct aiVector3D m_sceneMax;
    qreal m_scaleFactor;

};

#endif // MODEL_H
