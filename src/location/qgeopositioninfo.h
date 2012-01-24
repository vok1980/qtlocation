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
#ifndef QGEOPOSITIONINFO_H
#define QGEOPOSITIONINFO_H

#include "qgeocoordinate.h"

#include <QDateTime>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE



class QDebug;
class QDataStream;

class QGeoPositionInfoPrivate;
class Q_LOCATION_EXPORT QGeoPositionInfo
{
public:
    enum Attribute {
        Direction,
        GroundSpeed,
        VerticalSpeed,
        MagneticVariation,
        HorizontalAccuracy,
        VerticalAccuracy
    };

    QGeoPositionInfo();
    QGeoPositionInfo(const QGeoCoordinate &coordinate, const QDateTime &updateTime);
    QGeoPositionInfo(const QGeoPositionInfo &other);
    ~QGeoPositionInfo();

    QGeoPositionInfo &operator=(const QGeoPositionInfo &other);

    bool operator==(const QGeoPositionInfo &other) const;
    inline bool operator!=(const QGeoPositionInfo &other) const {
        return !operator==(other);
    }

    bool isValid() const;

    void setTimestamp(const QDateTime &timestamp);
    QDateTime timestamp() const;

    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate coordinate() const;

    void setAttribute(Attribute attribute, qreal value);
    qreal attribute(Attribute attribute) const;
    void removeAttribute(Attribute attribute);
    bool hasAttribute(Attribute attribute) const;

private:
#ifndef QT_NO_DEBUG_STREAM
    friend Q_LOCATION_EXPORT QDebug operator<<(QDebug dbg, const QGeoPositionInfo &info);
#endif
#ifndef QT_NO_DATASTREAM
    friend Q_LOCATION_EXPORT QDataStream &operator<<(QDataStream &stream, const QGeoPositionInfo &info);
    friend Q_LOCATION_EXPORT QDataStream &operator>>(QDataStream &stream, QGeoPositionInfo &info);
#endif
    QGeoPositionInfoPrivate *d;
};

#ifndef QT_NO_DEBUG_STREAM
Q_LOCATION_EXPORT QDebug operator<<(QDebug dbg, const QGeoPositionInfo &info);
#endif

#ifndef QT_NO_DATASTREAM
Q_LOCATION_EXPORT QDataStream &operator<<(QDataStream &stream, const QGeoPositionInfo &info);
Q_LOCATION_EXPORT QDataStream &operator>>(QDataStream &stream, QGeoPositionInfo &info);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
