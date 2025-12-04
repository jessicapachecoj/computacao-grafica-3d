#ifndef DISPLAYFILEWIDGET_H
#define DISPLAYFILEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QRect>
#include <QMatrix4x4>
#include <QPoint>
#include "geometria.h"

class DisplayFileWidget : public QWidget
{
    Q_OBJECT

signals:
    void rotationRequested(double dx, double dy);

public:
    explicit DisplayFileWidget(QWidget *parent = nullptr);

    void setDisplayFile(const QVector<ObjetoVirtual> *objetos);
    void setViewport(const QRect& vp);
    void setViewMatrix(const QMatrix4x4& matrix);
    void setProjectionMatrix(const QMatrix4x4& matrix);

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum OutCode {
        INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8
    };

    OutCode computeOutCode(double x, double y);
    bool cohenSutherlandClip(QPointF &p1, QPointF &p2);

    // Novo método para Clipping Volumétrico (3D) no espaço NDC [-1, 1].
    bool clip3D(QVector3D &p1, QVector3D &p2);

    const QVector<ObjetoVirtual> *displayFilePtr = nullptr;
    QRect m_viewport;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_projectionMatrix;

    QPoint lastMousePos;
    bool isDragging = false;
};

#endif // DISPLAYFILEWIDGET_H
