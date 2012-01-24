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
** This file is part of the Ovi services plugin for the Maps and
** Navigation API.  The use of these services, whether by use of the
** plugin or by other means, is governed by the terms and conditions
** described by the file OVI_SERVICES_TERMS_AND_CONDITIONS.txt in
** this package, located in the directory containing the Ovi services
** plugin source code.
**
****************************************************************************/

#include "qplacecontentreplyimpl.h"

#if defined(QT_PLACES_LOGGING)
    #include <QDebug>
#endif

QT_USE_NAMESPACE

/*!
    Constructor.
*/
QPlaceContentReplyImpl::QPlaceContentReplyImpl(const QPlaceContentRequest &request, QPlaceRestReply *reply,
                                               QPlaceManager *manager, QObject *parent)
:   QPlaceContentReply(parent), restReply(reply), startNumber(0)

{
    setRequest(request);
    if (request.contentType() == QPlaceContent::ImageType)
        parser = new QPlaceJSonMediaParser(this);
    else if (request.contentType() == QPlaceContent::ReviewType)
        parser = new QPlaceJSonReviewParser(manager, this);
    else if (request.contentType() == QPlaceContent::EditorialType)
        parser = new QPlaceJSonDetailsParser(manager, this);
    else
        parser = 0;

    if (restReply) {
        restReply->setParent(this);
        connect(restReply, SIGNAL(finished(const QString &)),
                parser, SLOT(processData(const QString &)));
        connect(restReply, SIGNAL(error(QPlaceRestReply::Error)),
                this, SLOT(restError(QPlaceRestReply::Error)));
        connect(parser, SIGNAL(finished(QPlaceJSonParser::Error,QString)),
                this, SLOT(resultReady(QPlaceJSonParser::Error,QString)));
    }
}

/*!
    Destructor.
*/
QPlaceContentReplyImpl::~QPlaceContentReplyImpl()
{
}

void QPlaceContentReplyImpl::abort()
{
    if (restReply)
        restReply->cancelProcessing();
}

void QPlaceContentReplyImpl::setStartNumber(int number)
{
    startNumber = number;
}

void QPlaceContentReplyImpl::restError(QPlaceReply::Error errorId, const QString &errorString)
{
    setError(errorId, errorString);

    emit error(this->error(), this->errorString());
    emit processingError(this, this->error(), this->errorString());
    setFinished(true);
    emit finished();
    emit processingFinished(this);
}

void QPlaceContentReplyImpl::restError(QPlaceRestReply::Error errorId)
{
    if (errorId == QPlaceRestReply::Canceled) {
        this->setError(CancelError, "RequestCanceled");
    } else if (errorId == QPlaceRestReply::NetworkError) {
        this->setError(CommunicationError, "Network error");
    }
    emit error(this->error(), this->errorString());
    emit processingError(this, this->error(), this->errorString());
    setFinished(true);
    emit finished();
    emit processingFinished(this);
}

void QPlaceContentReplyImpl::resultReady(const QPlaceJSonParser::Error &errorId,
                      const QString &errorMessage)
{
    if (errorId == QPlaceJSonParser::NoError) {
        if (request().contentType() == QPlaceContent::ImageType) {
            QPlaceJSonMediaParser * mediaParser = qobject_cast<QPlaceJSonMediaParser*>(parser);
            QList<QPlaceImage> imageOjects = mediaParser->resultMedia();
            QPlaceContent::Collection collection;
            for (int i=0; i < imageOjects.count(); ++i)
                collection.insert(startNumber +i, imageOjects.at(i));
            setContent(collection);
            setTotalCount(mediaParser->allMediaCount());
        } else if (request().contentType() == QPlaceContent::ReviewType) {
            QPlaceJSonReviewParser *reviewParser = qobject_cast<QPlaceJSonReviewParser*>(parser);
            QList<QPlaceReview> reviewObjects = reviewParser->results();
            QPlaceContent::Collection collection;
            for (int i=0; i < reviewObjects.count(); ++i)
                collection.insert(startNumber + i, reviewObjects.at(i));
            setContent(collection);
            setTotalCount(reviewParser->allReviewsCount());
        } else if (request().contentType() == QPlaceContent::EditorialType) {
            QPlaceJSonDetailsParser *detailsParser =
                qobject_cast<QPlaceJSonDetailsParser *>(parser);

            const QPlace place = detailsParser->result();
            QPlaceContent::Collection collection = place.content(QPlaceContent::EditorialType);
            setContent(collection);
            setTotalCount(collection.count());
        }
    } else if (errorId == QPlaceJSonParser::ParsingError) {
        setError(ParseError, errorMessage);
        emit error(this->error(), this->errorString());
        emit processingError(this, ParseError, errorMessage);
    }
    setFinished(true);
    emit finished();
    emit processingFinished(this);
    parser->deleteLater();
    parser = NULL;
    restReply->deleteLater();
    restReply = NULL;
}
