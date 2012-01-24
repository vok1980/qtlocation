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

#include "qplacecontent.h"
#include "qplacecontent_p.h"

#include <QtCore/QUrl>

#if !defined(Q_CC_MWERKS)
template<> QT_PREPEND_NAMESPACE(QPlaceContentPrivate) *QSharedDataPointer<QT_PREPEND_NAMESPACE(QPlaceContentPrivate)>::clone()
{
    return d->clone();
}
#endif

QT_USE_NAMESPACE

inline QPlaceContentPrivate *QPlaceContent::d_func()
{
    return static_cast<QPlaceContentPrivate *>(d_ptr.data());
}

inline const QPlaceContentPrivate *QPlaceContent::d_func() const
{
    return static_cast<const QPlaceContentPrivate *>(d_ptr.constData());
}

bool QPlaceContentPrivate::compare(const QPlaceContentPrivate *other) const
{
    return supplier == other->supplier
            && user == other->user
            && attribution == other->attribution;
}

/*!
    \class QPlaceContent
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-data
    \since QtLocation 5.0

    \brief The QPlaceContent class serves as the base class for rich content types.

    Rich content such as \l {QPlaceImage}{images}, \l {QPlaceReview}{reviews}
    and \l {QPlaceEditorial}{editorials} inherit
    from the QPlaceContent class which contains common properties such as
    an attribution string and content contributor, which may take the form of a
    \l {QPlaceUser}{user} and/or \l {QPlaceSupplier}{supplier}.  It is possible that
    a user from a supplier is contributing content, hence both fields could
    be filled in simultaneously.

    \bold {Note:} Some providers may \e {require} that the attribution string be displayed
    to the user whenever a piece of content is viewed.

    Conversion between QPlaceContent and it's subclasses can be easily performed without
    casting. Due to the way it has been implemented, object slicing is not an issue,
    the following code is valid:
    \snippet snippets/places/requesthandler.h Content conversion

    The rich content of a place is typically made available as paginated items.  The ability
    to convert between QPlaceContent and it's subclasses means that code which handles
    the mechanics of paging can be easily shared for each of the sub types.

    At present the QPlaceContent class is not extensible by 3rd parties.

    Note:  The Places API considers content objects to be 'retrieve-only' objects.
    Submission of content to a provider is not a supported use case.
    \sa QPlaceImage, QPlaceReview, QPlaceEditorial
*/

/*!
    \typedef QPlaceContent::Collection
    Synonym for QMap<int, QPlaceContent>.  The key of the map is an \c int representing the
    index of the content.  The value is the content object itself.

    The \c {Collection} is intended to be a container where content items, that have been retrieved
    as pages, can be stored.  This enables a developer to skip pages, e.g. indexes 0-9 may be
    stored in the collection,  if the user skips to indexes 80-99, these can be stored in the
    collection as well.
*/

/*!
    \enum QPlaceContent::Type
    Defines the type of content.
    \value NoType
        The content object is default constructed, any other content type may be assigned
        to this content object.
    \value ImageType
        The content object is an image.
    \value ReviewType
        The content object is a review.
    \value EditorialType
        The content object is an editorial
*/

/*!
    Constructs an default content object which has no type.
*/
QPlaceContent::QPlaceContent()
    :d_ptr(0)
{
}

/*!
    Constructs a new copy of \a other.
*/
QPlaceContent::QPlaceContent(const QPlaceContent &other)
    :d_ptr(other.d_ptr)
{
}

/*!
    Assigns the \a other content object to this and returns a reference
    to this content object.
*/
QPlaceContent &QPlaceContent::operator=(const QPlaceContent &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Destroys the content object.
*/
QPlaceContent::~QPlaceContent()
{
}

/*!
    Returns the content type.
*/
QPlaceContent::Type QPlaceContent::type() const
{
    if (!d_ptr)
        return NoType;
    return d_ptr->type();
}

/*!
    Returns true if this content object is equivalent to \a other,
    otherwise returns false.
*/
bool QPlaceContent::operator==(const QPlaceContent &other) const
{
    // An invalid content object is only equal to another invalid content object
    if (!d_ptr)
        return !other.d_ptr;

    if (type() != other.type())
        return false;

    return d_ptr->compare(other.d_ptr);
}

/*!
    Returns true if this content object is not equivalent to \a other,
    otherwise returns false.
*/
bool QPlaceContent::operator!=(const QPlaceContent &other) const
{
    return !(*this == other);
}

/*!
    Returns the supplier who contributed this content.
*/
QPlaceSupplier QPlaceContent::supplier() const
{
    Q_D(const QPlaceContent);

    return d->supplier;
}

/*!
    Sets the \a supplier of the content.
*/
void QPlaceContent::setSupplier(const QPlaceSupplier &supplier)
{
    Q_D(QPlaceContent);

    d->supplier = supplier;
}

/*!
    Returns the user who contributed this content.
*/
QPlaceUser QPlaceContent::user() const
{
    Q_D(const QPlaceContent);
    return d->user;
}

/*!
    Sets the \a user who contributed this content.
*/
void QPlaceContent::setUser(const QPlaceUser &user)
{
    Q_D(QPlaceContent);
    d->user = user;
}

/*!
    Returns a rich text attribution string.

    \bold {Note}: Some providers may require that the attribution
    of a particular content item always be displayed
    when the content item is shown.
*/
QString QPlaceContent::attribution() const
{
    Q_D(const QPlaceContent);
    return d->attribution;
}

/*!
    Sets a rich text \a attribution string for this content item.
*/
void QPlaceContent::setAttribution(const QString &attribution)
{
    Q_D(QPlaceContent);
    d->attribution = attribution;
}

/*!
    \internal
    Constructs a new content object from the given pointer \a d.
*/
QPlaceContent::QPlaceContent(QPlaceContentPrivate *d)
    :d_ptr(d)
{
}
