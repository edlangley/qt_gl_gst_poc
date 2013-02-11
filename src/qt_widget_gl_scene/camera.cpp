
#include "camera.h"

QMatrix4x4 fromRotation(float angle, Qt::Axis axis)
{
    QMatrix4x4 m;
    if (axis == Qt::XAxis)
        m.rotate(angle, QVector3D(1, 0, 0));
    else if (axis == Qt::YAxis)
        m.rotate(-angle, QVector3D(0, 1, 0));
    else if (axis == Qt::ZAxis)
        m.rotate(angle, QVector3D(0, 0, 1));
    return m;
}

QMatrix4x4 fromProjection(float fovAngle)
{
    float fov = qCos(fovAngle / 2) / qSin(fovAngle / 2);

    float zNear = 0.01;
    float zFar = 1000;

    float m33 = (zNear + zFar) / (zNear - zFar);
    float m34 = (2 * zNear * zFar) / (zNear - zFar);

    qreal data[] =
    {
        fov, 0, 0, 0,
        0, fov, 0, 0,
        0, 0, m33, m34,
        0, 0, -1, 0
    };
    return QMatrix4x4(data);
}

QTransform rotatingTransform(qreal angle)
{
    QTransform transform;
    transform.rotate(angle);
    return transform;
}

void Camera::setPitch(qreal pitch)
{
    m_pitch = qBound(qreal(-30), pitch, qreal(30));
    m_matrixDirty = true;
}

void Camera::setYaw(qreal yaw)
{
    m_yaw = yaw;
    m_matrixDirty = true;
}

void Camera::setPos(const QPointF &pos)
{
    m_pos = pos;
    m_matrixDirty = true;
}

void Camera::setFov(qreal fov)
{
    m_fov = fov;
    m_matrixDirty = true;
}

void Camera::setTime(qreal time)
{
    m_time = time;
    m_matrixDirty = true;
}


const QMatrix4x4 &Camera::viewMatrix() const
{
    updateMatrix();
    return m_viewMatrix;
}

const QMatrix4x4 &Camera::viewProjectionMatrix() const
{
    updateMatrix();
    return m_viewProjectionMatrix;
}

void Camera::updateMatrix() const
{
    if (!m_matrixDirty)
        return;

    m_matrixDirty = false;

    QMatrix4x4 m;
    m.scale(-1, 1, 1);
    m *= fromRotation(m_yaw + 180, Qt::YAxis);
//    m.translate(-m_pos.x(), 0.04 * qSin(10 * m_time) + 0.1, -m_pos.y());
    m.translate(-m_pos.x(), 0.1, -m_pos.y());
    m = fromRotation(m_pitch, Qt::XAxis) * m;
    m_viewMatrix = m;
    m_viewProjectionMatrix = fromProjection(m_fov) * m_viewMatrix;
}


