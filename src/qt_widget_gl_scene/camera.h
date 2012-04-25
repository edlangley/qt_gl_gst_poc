#ifndef CAMERA_H
#define CAMERA_H

#include <qmath.h>
#include <QMatrix4x4>
#include <QTransform>

QMatrix4x4 fromRotation(float angle, Qt::Axis axis);
QMatrix4x4 fromProjection(float fovAngle);
QTransform rotatingTransform(qreal angle);


class Camera
{
public:
    Camera()
        : m_yaw(0)
        , m_pitch(0)
        , m_fov(70)
        , m_time(0)
        , m_matrixDirty(true)
    {
    }

    qreal yaw() const { return m_yaw; }
    qreal pitch() const { return m_pitch; }
    qreal fov() const { return m_fov; }
    QPointF pos() const { return m_pos; }

    void setYaw(qreal yaw);
    void setPitch(qreal pitch);
    void setPos(const QPointF &pos);
    void setFov(qreal fov);
    void setTime(qreal time);

    const QMatrix4x4 &viewProjectionMatrix() const;
    const QMatrix4x4 &viewMatrix() const;

private:
    void updateMatrix() const;

    qreal m_yaw;
    qreal m_pitch;
    qreal m_fov;
    qreal m_time;

    QPointF m_pos;

    mutable bool m_matrixDirty;
    mutable QMatrix4x4 m_viewMatrix;
    mutable QMatrix4x4 m_viewProjectionMatrix;
};

#endif // CAMERA_H
