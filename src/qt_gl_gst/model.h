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
    QVector<QVector3D> triangleVertices;
    bool hasNormals;
    QVector<QVector3D> triangleNormals;
    bool hasTexcoords;
    QVector<QVector2D> triangleTexcoords;

    // Could add more QVectors here for points, lines, polys.

    // texture related members here ....

signals:

public slots:

};

class ModelNode
{
public:
    QVector<ModelMesh> meshes;
    struct aiMatrix4x4 transformMatrix;
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
    void Draw(QMatrix4x4 MVPMatrix, QGLShaderProgram *shaderProg, bool useModelTextures);

private:
    void aiNodesToVertexArrays();
    void get_bounding_box_for_node (const struct aiNode* nd,
            struct aiVector3D* min,
            struct aiVector3D* max,
            struct aiMatrix4x4* trafo);
    void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);

    const struct aiScene* scene;
    QVector<ModelNode> nodes;
    struct aiVector3D scene_center;
    struct aiVector3D scene_min;
    struct aiVector3D scene_max;
    qreal scaleFactor;

};

#endif // MODEL_H
