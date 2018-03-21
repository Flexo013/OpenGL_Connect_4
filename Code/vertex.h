#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>

struct Vertex
{
    QVector3D coords;
    QVector3D normal;

    Vertex(QVector3D coords, QVector3D normal)
        :
          coords(coords),
          normal(normal)
    {    }
};

#endif // VERTEX_H

