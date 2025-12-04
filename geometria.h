#ifndef GEOMETRIA_H
#define GEOMETRIA_H

#include <QString>
#include <QVector>
#include <QList>
#include <QVector3D>
#include <QMatrix4x4>

struct ObjetoVirtual {
    QString nome;
    QVector<QVector3D> vertices;
    QVector<QList<int>> faces;
    QMatrix4x4 matrizModelo;
    QVector3D centro;
    ObjetoVirtual(const QString& n)
        : nome(n) {
        matrizModelo.setToIdentity();
        centro = QVector3D(0, 0, 0);
    }
    ObjetoVirtual() {
        matrizModelo.setToIdentity();
        centro = QVector3D(0, 0, 0);
    }
};

#endif // GEOMETRIA_H
