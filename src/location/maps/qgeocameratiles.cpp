/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qgeocameratiles_p.h"

#include "qgeocameradata_p.h"
#include "qgeoprojection_p.h"
#include "qgeotilespec.h"

#include "qdoublevector2d_p.h"
#include "qdoublevector3d_p.h"

#include "qgeomaptype.h"

#include <QVector>

#include <QDebug>

#include <cmath>

QT_BEGIN_NAMESPACE

struct Frustum {
    QDoubleVector3D topLeftNear;
    QDoubleVector3D topLeftFar;
    QDoubleVector3D topRightNear;
    QDoubleVector3D topRightFar;
    QDoubleVector3D bottomLeftNear;
    QDoubleVector3D bottomLeftFar;
    QDoubleVector3D bottomRightNear;
    QDoubleVector3D bottomRightFar;
};

typedef QVector<QDoubleVector3D> Polygon;

class QGeoCameraTilesPrivate {
public:
    QGeoCameraTilesPrivate(QSharedPointer<QGeoProjection> projection);
    ~QGeoCameraTilesPrivate();

    QSharedPointer<QGeoProjection> projection_;
    QString pluginString_;
    QGeoMapType mapType_;
    QGeoCameraData camera_;
    QSize screenSize_;
    int tileSize_;
    int maxZoom_;
    QSet<QGeoTileSpec> tiles_;

    void updateMetadata();
    void updateGeometry();

    double sideLength() const;

    Frustum frustum() const;

    class LengthSorter {
    public:
        QDoubleVector3D base;
        bool operator()(const QDoubleVector3D &lhs, const QDoubleVector3D &rhs) {
            return (lhs - base).lengthSquared() < (rhs - base).lengthSquared();
        }
    };

    void appendZIntersects(const QDoubleVector3D &start, const QDoubleVector3D &end, double z, QVector<QDoubleVector3D> &results) const;
    Polygon frustumFootprint(const Frustum &frustum) const;

    QPair<Polygon, Polygon> splitPolygonAtAxisValue(const Polygon &polygon, int axis, double value) const;
    QPair<Polygon, Polygon> clipFootprintToMap(const Polygon &footprint) const;

    QList<QPair<double, int> > tileIntersections(double p1, int t1, double p2, int t2) const;
    QSet<QGeoTileSpec> tilesFromPolygon(const Polygon &polygon) const;

    struct TileMap
    {
        TileMap(int minY, int maxY);

        int size;
        int minY;
        int maxY;
        QVector<int> minX;
        QVector<int> maxX;

        void add(int tileX, int tileY);
    };
};

QGeoCameraTiles::QGeoCameraTiles(QSharedPointer<QGeoProjection> projection)
    : d_ptr(new QGeoCameraTilesPrivate(projection)) {}

QGeoCameraTiles::~QGeoCameraTiles()
{
    delete d_ptr;
}

void QGeoCameraTiles::setCamera(const QGeoCameraData &camera)
{
    Q_D(QGeoCameraTiles);

    if (d->camera_ == camera)
        return;

    d->camera_ = camera;
    d->updateGeometry();
}

void QGeoCameraTiles::setScreenSize(const QSize &size)
{
    Q_D(QGeoCameraTiles);

    if (d->screenSize_ == size)
        return;

    d->screenSize_ = size;
    d->updateGeometry();
}

void QGeoCameraTiles::setPluginString(const QString &pluginString)
{
    Q_D(QGeoCameraTiles);

    if (d->pluginString_ == pluginString)
        return;

    d->pluginString_ = pluginString;
    d->updateMetadata();
}

void QGeoCameraTiles::setMapType(const QGeoMapType &mapType)
{
    Q_D(QGeoCameraTiles);

    if (d->mapType_ == mapType)
        return;

    d->mapType_ = mapType;
    d->updateMetadata();
}

void QGeoCameraTiles::setTileSize(int tileSize)
{
    Q_D(QGeoCameraTiles);

    if (d->tileSize_ == tileSize)
        return;

    d->tileSize_ = tileSize;
    d->updateGeometry();
}

void QGeoCameraTiles::setMaximumZoomLevel(int maxZoom)
{
    Q_D(QGeoCameraTiles);

    if (d->maxZoom_ == maxZoom)
        return;

    d->maxZoom_ = maxZoom;
    d->updateGeometry();
}

QSet<QGeoTileSpec> QGeoCameraTiles::tiles() const
{
    Q_D(const QGeoCameraTiles);

    return d->tiles_;
}

QGeoCameraTilesPrivate::QGeoCameraTilesPrivate(QSharedPointer<QGeoProjection> projection)
    : projection_(projection) {}

QGeoCameraTilesPrivate::~QGeoCameraTilesPrivate() {}

void QGeoCameraTilesPrivate::updateMetadata()
{
    typedef QSet<QGeoTileSpec>::const_iterator iter;

    QSet<QGeoTileSpec> newTiles;

    iter i = tiles_.constBegin();
    iter end = tiles_.constEnd();

    for (; i != end; ++i) {
        QGeoTileSpec tile = *i;
        newTiles.insert(QGeoTileSpec(pluginString_, mapType_.mapId(), tile.zoom(), tile.x(), tile.y()));
    }

    tiles_ = newTiles;
}

void QGeoCameraTilesPrivate::updateGeometry()
{
    tiles_.clear();

    // Find the frustum from the camera / screen / viewport information
    Frustum f = frustum();

    // Find the polygon where the frustum intersects the plane of the map
    Polygon footprint = frustumFootprint(f);

    // Clip the polygon to the map, split it up if it cross the dateline
    QPair<Polygon, Polygon> polygons = clipFootprintToMap(footprint);

    if (!polygons.first.isEmpty()) {
        tiles_.unite(tilesFromPolygon(polygons.first));
    }

    if (!polygons.second.isEmpty()) {
        tiles_.unite(tilesFromPolygon(polygons.second));
    }
}

double QGeoCameraTilesPrivate::sideLength() const
{
    return double(1 << camera_.zoomLevel());
}

Frustum QGeoCameraTilesPrivate::frustum() const
{
    int zpow2 = 1 << camera_.zoomLevel();

    QDoubleVector3D center = zpow2 * projection_->coordToMercator(camera_.center());
    center.setZ(0.0);

    double f = qMin(screenSize_.width(), screenSize_.height()) / (1.0 * tileSize_);

    double z = pow(2.0, camera_.zoomFactor() - camera_.zoomLevel());

    double altitude = f / (2.0 * z);
    QDoubleVector3D eye = center;
    eye.setZ(altitude);

    QDoubleVector3D view = eye - center;
    QDoubleVector3D side = QDoubleVector3D::normal(view, QDoubleVector3D(0.0, 1.0, 0.0));
    QDoubleVector3D up = QDoubleVector3D::normal(side, view);

    double nearPlane = zpow2 / (1.0 * tileSize_ * (1 << maxZoom_));
    double farPlane = 3.0;

    double aspectRatio = 1.0 * screenSize_.width() / screenSize_.height();

    double hn = 0.0;
    double wn = 0.0;
    double hf = 0.0;
    double wf = 0.0;

    // fixes field of view at 45 degrees
    // this assumes that viewSize = 2*nearPlane x 2*nearPlane

    if (aspectRatio > 1.0) {
        hn = 2 * nearPlane;
        wn = hn * aspectRatio;

        hf = 2 * farPlane;
        wf = hf * aspectRatio;
    } else {
        wn = 2 * nearPlane;
        hn = wn / aspectRatio;

        wf = 2 * farPlane;
        hf = wf / aspectRatio;
    }

    QDoubleVector3D d = center - eye;
    d.normalize();
    up.normalize();
    QDoubleVector3D right = QDoubleVector3D::normal(d, up);

    QDoubleVector3D cf = eye + d * farPlane;
    QDoubleVector3D cn = eye + d * nearPlane;

    Frustum frustum;

    frustum.topLeftFar = cf + (up * hf / 2) - (right * wf / 2);
    frustum.topRightFar = cf + (up * hf / 2) + (right * wf / 2);
    frustum.bottomLeftFar = cf - (up * hf / 2) - (right * wf / 2);
    frustum.bottomRightFar = cf - (up * hf / 2) + (right * wf / 2);

    frustum.topLeftNear = cn + (up * hn / 2) - (right * wn / 2);
    frustum.topRightNear = cn + (up * hn / 2) + (right * wn / 2);
    frustum.bottomLeftNear = cn - (up * hn / 2) - (right * wn / 2);
    frustum.bottomRightNear = cn - (up * hn / 2) + (right * wn / 2);

    return frustum;
}

void QGeoCameraTilesPrivate::appendZIntersects(const QDoubleVector3D &start,
                                               const QDoubleVector3D &end,
                                               double z,
                                               QVector<QDoubleVector3D> &results) const
{
    if (start.z() == end.z()) {
        if (start.z() == z) {
            results.append(start);
            results.append(end);
        }
    } else {
        double f = (start.z() - z) / (start.z() - end.z());
        if ((0 <= f) && (f <= 1.0)) {
            results.append((1 - f) * start + f * end);
        }
    }
}

// Returns the intersection of the plane of the map and the camera frustum as a right handed polygon
Polygon QGeoCameraTilesPrivate::frustumFootprint(const Frustum &frustum) const
{
    Polygon points;
    points.reserve(24);

    appendZIntersects(frustum.topLeftNear, frustum.topLeftFar, 0.0, points);
    appendZIntersects(frustum.topRightNear, frustum.topRightFar, 0.0, points);
    appendZIntersects(frustum.bottomLeftNear, frustum.bottomLeftFar, 0.0, points);
    appendZIntersects(frustum.bottomRightNear, frustum.bottomRightFar, 0.0, points);

    appendZIntersects(frustum.topLeftNear, frustum.bottomLeftNear, 0.0, points);
    appendZIntersects(frustum.bottomLeftNear, frustum.bottomRightNear, 0.0, points);
    appendZIntersects(frustum.bottomRightNear, frustum.topRightNear, 0.0, points);
    appendZIntersects(frustum.topRightNear, frustum.topLeftNear, 0.0, points);

    appendZIntersects(frustum.topLeftFar, frustum.bottomLeftFar, 0.0, points);
    appendZIntersects(frustum.bottomLeftFar, frustum.bottomRightFar, 0.0, points);
    appendZIntersects(frustum.bottomRightFar, frustum.topRightFar, 0.0, points);
    appendZIntersects(frustum.topRightFar, frustum.topLeftFar, 0.0, points);

    if (points.isEmpty())
        return points;

    // sort points into a right handed polygon

    LengthSorter sorter;

    // - initial sort to remove duplicates
    sorter.base = points.first();
    qSort(points.begin(), points.end(), sorter);
    for (int i = points.size() - 1; i > 0; --i) {
        if (points.at(i) == points.at(i - 1))
            points.remove(i);
    }

    // - proper sort
    //   - start with the first point, put it in the sorted part of the list
    //   - add the nearest unsorted point to the last sorted point to the end
    //     of the sorted points
    Polygon::iterator i;
    for (i = points.begin(); i != points.end(); ++i) {
        sorter.base = *i;
        if (i + 1 != points.end())
            qSort(i + 1, points.end(), sorter);
    }

    // - determine if what we have is right handed
    int size = points.size();
    if (size >= 3) {
        QDoubleVector3D normal = QDoubleVector3D::normal(points.at(1) - points.at(0),
                                                         points.at(2) - points.at(1));
        // - if not, reverse the list
        if (normal.z() < 0.0) {
            int halfSize = size / 2;
            for (int i = 0; i < halfSize; ++i) {
                QDoubleVector3D spare = points.at(i);
                points[i] = points[size - 1 - i];
                points[size - 1 - i] = spare;
            }
        }
    }

    return points;
}

QPair<Polygon, Polygon> QGeoCameraTilesPrivate::splitPolygonAtAxisValue(const Polygon &polygon, int axis, double value) const
{
    Polygon polygonBelow;
    Polygon polygonAbove;

    int size = polygon.size();

    if (size == 0) {
        return QPair<Polygon, Polygon>(polygonBelow, polygonAbove);
    }

    QVector<int> comparisons = QVector<int>(polygon.size());

    for (int i = 0; i < size; ++i) {
        double v = polygon.at(i).get(axis);
        if (qFuzzyCompare(v - value + 1.0, 1.0)) {
            comparisons[i] = 0;
        } else {
            if (v < value) {
                comparisons[i] = -1;
            } else if (value < v) {
                comparisons[i] = 1;
            }
        }
    }

    for (int index = 0; index < size; ++index) {
        int prevIndex = index - 1;
        if (prevIndex < 0)
            prevIndex += size;
        int nextIndex = (index + 1) % size;

        int prevComp = comparisons[prevIndex];
        int comp = comparisons[index];
        int nextComp = comparisons[nextIndex];

         if (comp == 0) {
            if (prevComp == -1) {
                polygonBelow.append(polygon.at(index));
                if (nextComp == 1) {
                    polygonAbove.append(polygon.at(index));
                }
            } else if (prevComp == 1) {
                polygonAbove.append(polygon.at(index));
                if (nextComp == -1) {
                    polygonBelow.append(polygon.at(index));
                }
            } else if (prevComp == 0) {
                if (nextComp == -1) {
                    polygonBelow.append(polygon.at(index));
                } else if (nextComp == 1) {
                    polygonAbove.append(polygon.at(index));
                } else if (nextComp == 0) {
                    // do nothing
                }
            }
        } else {
             if (comp == -1) {
                 polygonBelow.append(polygon.at(index));
             } else if (comp == 1) {
                 polygonAbove.append(polygon.at(index));
             }

             // there is a point between this and the next point
             // on the polygon that lies on the splitting line
             // and should be added to both the below and above
             // polygons
             if ((nextComp != 0) && (nextComp != comp)) {
                 QDoubleVector3D p1 = polygon.at(index);
                 QDoubleVector3D p2 = polygon.at(nextIndex);

                 double p1v = p1.get(axis);
                 double p2v = p2.get(axis);

                 double f = (p1v - value) / (p1v - p2v);

                 if (((0 <= f) && (f <= 1.0))
                         || qFuzzyCompare(f + 1.0, 1.0)
                         || qFuzzyCompare(f + 1.0, 2.0) ) {
                     QDoubleVector3D midPoint = (1.0 - f) * p1 + f * p2;
                     polygonBelow.append(midPoint);
                     polygonAbove.append(midPoint);
                 }
             }
        }
    }

    return QPair<Polygon, Polygon>(polygonBelow, polygonAbove);
}


QPair<Polygon, Polygon> QGeoCameraTilesPrivate::clipFootprintToMap(const Polygon &footprint) const
{
    bool clipX0 = false;
    bool clipX1 = false;
    bool clipY0 = false;
    bool clipY1 = false;

    double side = sideLength();

    typedef Polygon::iterator iter;
    typedef Polygon::const_iterator const_iter;

    const_iter i = footprint.constBegin();
    const_iter end = footprint.constEnd();
    for (; i != end; ++i) {
        QDoubleVector3D p = *i;
        if (p.x() < 0.0)
            clipX0 = true;
        if (side < p.x())
            clipX1 = true;
        if (p.y() < 0.0)
            clipY0 = true;
        if (side < p.y())
            clipY1 = true;
    }

    Polygon results = footprint;

    if (clipY0) {
        results = splitPolygonAtAxisValue(results, 1, 0.0).second;
    }

    if (clipY1) {
        results = splitPolygonAtAxisValue(results, 1, side).first;
    }

    if (clipX0) {
        if (clipX1) {
            results = splitPolygonAtAxisValue(results, 0, 0.0).second;
            results = splitPolygonAtAxisValue(results, 0, side).first;
            return QPair<Polygon, Polygon>(results, Polygon());
        } else {
            QPair<Polygon, Polygon> pair = splitPolygonAtAxisValue(results, 0, 0.0);
            for (int i = 0; i < pair.first.size(); ++i) {
                pair.first[i].setX(pair.first.at(i).x() + side);
            }
            return pair;
        }
    } else {
        if (clipX1) {
            QPair<Polygon, Polygon> pair = splitPolygonAtAxisValue(results, 0, side);
            for (int i = 0; i < pair.second.size(); ++i) {
                pair.second[i].setX(pair.second.at(i).x() - side);
            }
            return pair;
        } else {
            return QPair<Polygon, Polygon>(results, Polygon());
        }
    }

}

QList<QPair<double, int> > QGeoCameraTilesPrivate::tileIntersections(double p1, int t1, double p2, int t2) const
{
    if (t1 == t2) {
        QList<QPair<double, int> > results = QList<QPair<double, int> >();
        results.append(QPair<double, int>(0.0, t1));
        return results;
    }

    int step = 1;
    if (t1 > t2) {
        step = -1;
    }

    int size = 1 + ((t2 - t1) / step);

    QList<QPair<double, int> > results = QList<QPair<double, int> >();

    results.append(QPair<double, int>(0.0, t1));

    if (step == 1) {
        for (int i = 1; i < size; ++i) {
            double f = (t1 + i - p1) / (p2 - p1);
            results.append(QPair<double, int>(f, t1 + i));
        }
    } else {
        for (int i = 1; i < size; ++i) {
            double f = (t1 - i + 1 - p1) / (p2 - p1);
            results.append(QPair<double, int>(f, t1 - i));
        }
    }

    return results;
}

QSet<QGeoTileSpec> QGeoCameraTilesPrivate::tilesFromPolygon(const Polygon &polygon) const
{
    int numPoints = polygon.size();

    if (numPoints == 0)
        return QSet<QGeoTileSpec>();

    int zoomLevel = camera_.zoomLevel();

    int minY = -1;
    int maxY = -1;

    int zpow2 = 1 << zoomLevel;

    QVector<int> tilesX(polygon.size());
    QVector<int> tilesY(polygon.size());

    double side = sideLength();

    for (int i = 0; i < numPoints; ++i) {

        QDoubleVector2D p = polygon.at(i).toVector2D();

        int x = 0;
        int y = 0;

        if (qFuzzyCompare(p.x(), side))
            x = zpow2 - 1;
        else
            x = static_cast<int>(p.x()) % zpow2;

        if (qFuzzyCompare(p.y(), side))
            y = zpow2 - 1;
        else
            y = static_cast<int>(p.y()) % zpow2;

        if (minY == -1) {
            minY = y;
            maxY = y;
        } else {
            minY = qMin(minY, y);
            maxY = qMax(maxY, y);
        }

        tilesX[i] = x;
        tilesY[i] = y;
    }

    QGeoCameraTilesPrivate::TileMap map(minY, maxY);

    for (int i1 = 0; i1 < numPoints; ++i1) {
        int i2 = (i1 + 1) % numPoints;

        QList<QPair<double, int> > xIntersects
                = tileIntersections(polygon.at(i1).get(0),
                                    tilesX.at(i1),
                                    polygon.at(i2).get(0),
                                    tilesX.at(i2));

        QList<QPair<double, int> > yIntersects
                = tileIntersections(polygon.at(i1).get(1),
                                    tilesY.at(i1),
                                    polygon.at(i2).get(1),
                                    tilesY.at(i2));

        int x = xIntersects.takeFirst().second;
        int y = yIntersects.takeFirst().second;

        map.add(x, y);

        while (!xIntersects.isEmpty() && !yIntersects.isEmpty()) {
            QPair<double, int> nextX = xIntersects.first();
            QPair<double, int> nextY = yIntersects.first();
            if (nextX.first < nextY.first) {
                x = nextX.second;
                map.add(x, y);
                xIntersects.removeFirst();
            } else if (nextX.first > nextY.first) {
                y = nextY.second;
                map.add(x, y);
                yIntersects.removeFirst();
            } else {
                map.add(x, nextY.second);
                map.add(nextX.second, y);
                x = nextX.second;
                y = nextY.second;
                map.add(x, y);
                xIntersects.removeFirst();
                yIntersects.removeFirst();
            }
        }

        while (!xIntersects.isEmpty()) {
            x = xIntersects.takeFirst().second;
            map.add(x, y);
        }

        while (!yIntersects.isEmpty()) {
            y = yIntersects.takeFirst().second;
            map.add(x, y);
        }
    }

    QSet<QGeoTileSpec> results;

    int size = map.minX.size();
    for (int i = 0; i < size; ++i) {
        int y = map.minY + i;
        int minX = map.minX[i];
        int maxX = map.maxX[i];
        for (int x = minX; x <= maxX; ++x) {
            results.insert(QGeoTileSpec(pluginString_, mapType_.mapId(), zoomLevel, x, y));
        }
    }

    return results;
}

QGeoCameraTilesPrivate::TileMap::TileMap(int minY, int maxY)
    : size(0),
      minY(minY),
      maxY(maxY),
      minX(maxY - minY + 1, -1),
      maxX(maxY - minY + 1, -1)
{}

void QGeoCameraTilesPrivate::TileMap::add(int tileX, int tileY)
{
    int index = tileY - minY;
    int min = minX.at(index);
    int max = maxX.at(index);

    if (min == -1) {
        min = tileX;
        max = tileX;
        minX[index] = min;
        maxX[index] = max;
        size += 1;
    } else {
        int oldSize = (max - min);
        int min2 = qMin(min, tileX);
        if (min2 != min)
            minX[index] = min2;
        int max2 = qMax(max, tileX);
        if (max2 != max)
            maxX[index] = max2;
        int newSize = (max2 - min2);
        size += (newSize - oldSize);
    }
}

QT_END_NAMESPACE
