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

#ifndef QPLACESEARCHREPLY_H
#define QPLACESEARCHREPLY_H

#include "qplacereply.h"
#include "qplacesearchresult.h"
#include "qplacesearchrequest.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE



class QPlaceSearchReplyPrivate;
class Q_LOCATION_EXPORT QPlaceSearchReply : public QPlaceReply
{
    Q_OBJECT
public:
    explicit QPlaceSearchReply(QObject *parent = 0);
    ~QPlaceSearchReply();

    QPlaceReply::Type type() const;

    QList<QPlaceSearchResult> results() const;
    QPlaceSearchRequest request() const;

protected:
    void setResults(const QList<QPlaceSearchResult> &results);
    void setRequest(const QPlaceSearchRequest &request);
private:
    Q_DISABLE_COPY(QPlaceSearchReply)
    Q_DECLARE_PRIVATE(QPlaceSearchReply)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
