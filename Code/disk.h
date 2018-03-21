#ifndef DISK_H
#define DISK_H

#include <QVector3D>

struct Disk
{
    int keyFrameNumber;
    float x, y;
    bool yellowDisk;

    Disk() = default;

    Disk(int keyFrameNumber, float x, float y, bool yellowDisk)
        :
          keyFrameNumber(keyFrameNumber),
          x(x),
          y(y),
          yellowDisk(yellowDisk)
    {    }
};

#endif // DISK_H
