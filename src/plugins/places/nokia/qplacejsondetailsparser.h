/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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
** $QT_END_LICENSE$
**
** This file is part of the Ovi services plugin for the Maps and
** Navigation API.  The use of these services, whether by use of the
** plugin or by other means, is governed by the terms and conditions
** described by the file OVI_SERVICES_TERMS_AND_CONDITIONS.txt in
** this package, located in the directory containing the Ovi services
** plugin source code.
**
****************************************************************************/

#ifndef QPLACEJSONDETAILSPARSER_H
#define QPLACEJSONDETAILSPARSER_H

#include <QObject>
#include <QList>

#include <qgeoplace.h>
#include "qplacejsonparser_p.h"

class QScriptEngine;
class QScriptValue;

QTM_BEGIN_NAMESPACE

class QPlaceJSonDetailsParser : public QPlaceJSonParser
{
    Q_OBJECT
public:
    explicit QPlaceJSonDetailsParser(QObject *parent = 0);
    virtual ~QPlaceJSonDetailsParser();

    static QGeoPlace buildPlace(const QScriptValue &place);
    QGeoPlace result();

private:
    void processJSonData(const QScriptValue &sv);
    static void buildPlace(const QScriptValue &place, QGeoPlace *targetPlace);
    static void processMainProvider(const QScriptValue &place, QGeoPlace *targetPlace);
    static void processContacts(const QScriptValue &contacts, QGeoPlace *targetPlace);
    static void processCategories(const QScriptValue &categories, QGeoPlace *targetPlace);
    static QPlaceCategory processCategory(const QScriptValue &category);
    static void processRatings(const QScriptValue &ratings, QGeoPlace *targetPlace);
    static QPlaceRating *processRating(const QScriptValue &rating);
    static void processAddress(const QScriptValue &address, QPlaceLocation *location);
    static void processLocations(const QScriptValue &locations, QGeoPlace *targetPlace);
    static QPlaceLocation processLocation(const QScriptValue &location);
    static void processTags(const QScriptValue &locations, QGeoPlace *targetPlace);
    static void processNames(const QScriptValue &locations, QGeoPlace *targetPlace);
    static QPlaceAlternativeValue *processName(const QScriptValue &nameValue);
    static void processPremiumContents(const QScriptValue &content, QGeoPlace *targetPlace);
    static void processPremiumVersion(const QScriptValue &content, QGeoPlace *targetPlace);
    static void processPremiumContent(const QScriptValue &content, QGeoPlace *targetPlace);
    static void processPremiumContentDescription(const QScriptValue &content,
                                                 const QPlaceSupplier &supplier,
                                                 QGeoPlace *targetPlace);
    static void processPremiumContentMediaObjects(const QScriptValue &content,
                                                  const QPlaceSupplier &supplier,
                                                  QGeoPlace *targetPlace);
    static QPlaceMediaObject *processPremiumContentMediaObject(const QScriptValue &content);
    static void processAdContent(const QScriptValue &content, QGeoPlace *targetPlace);
    static void processAdContentPackages(const QScriptValue &content, QGeoPlace *targetPlace);
    static void processAdContentDescriptions(const QScriptValue &content, QGeoPlace *targetPlace);
    static QPlaceDescription *processAdContentDescription(const QScriptValue &content);
    static void processAdContentMediaObjects(const QScriptValue &content, QGeoPlace *targetPlace);
    static QPlaceMediaObject *processAdContentMediaObject(const QScriptValue &content);
    static void processAdContentPaymentMethods(const QScriptValue &content, QGeoPlace *targetPlace);
    static QString processAdContentPaymentMethod(const QScriptValue &content);
    static void processAdContentBusinessHours(const QScriptValue &content, QGeoPlace *targetPlace);
    static void processAdContentClosingsNotes(const QScriptValue &content, QGeoPlace *targetPlace);
    static QString processAdContentClosingsNote(const QScriptValue &content);
    static void processAdContentOpeningHours(const QScriptValue &content, QGeoPlace *targetPlace);
    static QPlaceWeekdayHours *processAdContentOpeningHoursElement(const QScriptValue &content);
    static void processAdContentOpeningNotes(const QScriptValue &content, QGeoPlace *targetPlace);
    static QString processAdContentOpeningNote(const QScriptValue &content);

private:
    QGeoPlace place;
};

QTM_END_NAMESPACE

#endif // QPLACEJSONDETAILSPARSER_H
