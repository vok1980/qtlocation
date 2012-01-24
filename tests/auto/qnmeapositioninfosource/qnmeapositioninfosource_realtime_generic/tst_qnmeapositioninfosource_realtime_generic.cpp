/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
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

//TESTED_COMPONENT=src/location

#include "tst_qnmeapositioninfosource.h"

class tst_QNmeaPositionInfoSource_RealTime_Generic : public TestQGeoPositionInfoSource
{
    Q_OBJECT

public:
    tst_QNmeaPositionInfoSource_RealTime_Generic()
    {
        m_factory = new QNmeaPositionInfoSourceProxyFactory;
    }

    ~tst_QNmeaPositionInfoSource_RealTime_Generic()
    {
        delete m_factory;
    }

protected:
    QGeoPositionInfoSource *createTestSource()
    {
        QNmeaPositionInfoSource *source = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode);
        QNmeaPositionInfoSourceProxy *proxy = static_cast<QNmeaPositionInfoSourceProxy*>(m_factory->createProxy(source));
        Feeder *feeder = new Feeder(source);
        feeder->start(proxy);
        return source;
    }

private:
    QNmeaPositionInfoSourceProxyFactory *m_factory;
};

#include "tst_qnmeapositioninfosource_realtime_generic.moc"

QTEST_GUILESS_MAIN(tst_QNmeaPositionInfoSource_RealTime_Generic);
