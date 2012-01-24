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

#include "qplacesearchrequest.h"
#include "qgeocoordinate.h"
#include "qgeoboundingarea.h"

#include <QtCore/QSharedData>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QPlaceSearchRequestPrivate : public QSharedData
{
public:
    QPlaceSearchRequestPrivate();
    QPlaceSearchRequestPrivate(const QPlaceSearchRequestPrivate &other);
    ~QPlaceSearchRequestPrivate();

    QPlaceSearchRequestPrivate &operator=(const QPlaceSearchRequestPrivate &other);
    bool operator==(const QPlaceSearchRequestPrivate &other) const;

    void clear();

    QString searchTerm;
    QList<QPlaceCategory> categories;
    QGeoBoundingArea  *searchArea;
    int dymNumber;
    QtLocation::VisibilityScope visibilityScope;
    QPlaceSearchRequest::RelevanceHint relevanceHint;
    int limit;
    int offset;
};

QPlaceSearchRequestPrivate::QPlaceSearchRequestPrivate()
    :   QSharedData(),
        searchArea(0), dymNumber(0),
        visibilityScope(QtLocation::UnspecifiedVisibility), relevanceHint(QPlaceSearchRequest::UnspecifiedHint),
        limit(-1), offset(0)
{
}

QPlaceSearchRequestPrivate::QPlaceSearchRequestPrivate(const QPlaceSearchRequestPrivate &other)
    : QSharedData(other),
      searchTerm(other.searchTerm),
      categories(other.categories),
      dymNumber(other.dymNumber),
      visibilityScope(other.visibilityScope),
      relevanceHint(other.relevanceHint),
      limit(other.limit),
      offset(other.offset)
{

    if (other.searchArea)
        searchArea = other.searchArea->clone();
    else
        searchArea = 0;
}

QPlaceSearchRequestPrivate::~QPlaceSearchRequestPrivate()
{
    delete searchArea;
}

QPlaceSearchRequestPrivate &QPlaceSearchRequestPrivate::operator=(const QPlaceSearchRequestPrivate &other)
{
    if (this != &other) {
        searchTerm = other.searchTerm;
        categories = other.categories;
        if (other.searchArea)
            searchArea = other.searchArea->clone();
        else
            searchArea = 0;
        dymNumber = other.dymNumber;
        visibilityScope = other.visibilityScope;
        relevanceHint = other.relevanceHint;
        limit = other.limit;
        offset = other.offset;
    }

    return *this;
}

bool QPlaceSearchRequestPrivate::operator==(const QPlaceSearchRequestPrivate &other) const
{
    bool searchAreaMatch = false;
    if ((searchArea == 0) && (other.searchArea == 0)) {
        searchAreaMatch = true;
    } else if (searchArea && other.searchArea) {
        if (*searchArea == *(other.searchArea))
            searchAreaMatch = true;
        else
            searchAreaMatch = false;
    } else {
        searchAreaMatch = false;
    }

    return (
            searchTerm == other.searchTerm
            && categories == other.categories
            && dymNumber == other.dymNumber
            && searchAreaMatch
            && visibilityScope == other.visibilityScope
            && relevanceHint == other.relevanceHint
            && limit == other.limit
            && offset == other.offset
    );
}

void QPlaceSearchRequestPrivate::clear()
{
    limit = -1;
    offset = 0;
    searchTerm.clear();
    categories.clear();
    delete searchArea;
    searchArea = 0;
    dymNumber = 0;
    visibilityScope = QtLocation::UnspecifiedVisibility;
    relevanceHint = QPlaceSearchRequest::UnspecifiedHint;
}

/*!
    \class QPlaceSearchRequest
    \inmodule QtLocation
    \ingroup QtLocation-places
    \ingroup QtLocation-places-requests
    \since QtLocation 5.0

    \brief The QPlaceSearchRequest class represents the set of parameters for a search request.

    A typical search request may look like the following:
    \snippet snippets/places/requesthandler.h Search request

    Note that specifying a search center can be done by setting a circular search area that has
    a center but no radius.    The default radius is set to -1, which indicates an undefined radius.  The provider will
    interpret this as being free to choose its own default radius.

    The QPlaceSearchRequest will assume ownership of the bounding area and will be responsible
    for its destruction.

    The QPlaceSearchRequest is primarily used with the QPlaceManager to
    \l {QPlaceManager::search()} {search for places}, however it is also
    used to provide parameters for \l {QPlaceManager::searchSuggestions()}{generating search term suggestions}
    and \l {QPlaceManager::recommendations()} {retreiving recommendations}.  Note that depending on usage,
    some parameters may not be relevant, e.g. the relevance hint is not important for search term suggestions.  However
    in general, most of the parameters are useful for each of these operations, eg for a recommendation, a search area
    and categories can be useful in narrowing down recommendation candidates.

    Also be aware that providers may vary by which parameters they support e.g. some providers may not support
    paging while others do, some providers may honor relevance hints while others may completely ignore them,
    see \l {Information about plugins}.

*/

/*!
    \enum QPlaceSearchRequest::RelevanceHint

    Defines hints to help rank place results.
    \value UnspecifiedHint
        No explicit hint has been specified.
    \value DistanceHint
        Distance to a search center is relevant for the user.  Closer places
        are more highly weighted.  This hint is only useful
        if a circular bounding area is used in the query.
    \value LexicalPlaceNameHint
        Alphabetic ordering of places according to name is relevant to the user.
*/

/*!
    Default constructor. Constructs an new request object.
*/
QPlaceSearchRequest::QPlaceSearchRequest()
    : d_ptr(new QPlaceSearchRequestPrivate())
{
}

/*!
    Constructs a copy of \a other.
*/
QPlaceSearchRequest::QPlaceSearchRequest(const QPlaceSearchRequest &other)
    : d_ptr(other.d_ptr)
{
}

/*!
    Destroys the request object.
*/
QPlaceSearchRequest::~QPlaceSearchRequest()
{
}

/*!
    Assigns \a other to this search request and returns a reference
    to this search request.
*/
QPlaceSearchRequest &QPlaceSearchRequest::operator= (const QPlaceSearchRequest & other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns true if \a other is equal to this search request,
    otherwise returns false.
*/
bool QPlaceSearchRequest::operator== (const QPlaceSearchRequest &other) const
{
    Q_D(const QPlaceSearchRequest);
    return *d == *other.d_func();
}

/*!
    Returns true if \a other is not equal to this search request,
    otherwise returns false.
*/
bool QPlaceSearchRequest::operator!= (const QPlaceSearchRequest &other) const
{
    Q_D(const QPlaceSearchRequest);
    return !(*d == *other.d_func());
}

/*!
    Returns the search term.
*/
QString QPlaceSearchRequest::searchTerm() const
{
    Q_D(const QPlaceSearchRequest);
    return d->searchTerm;
}

/*!
    Sets the search \a term.
*/
void QPlaceSearchRequest::setSearchTerm(const QString &term)
{
    Q_D(QPlaceSearchRequest);
    d->searchTerm = term;
}

/*!
    Return the categories to be used in the search request.
    Places need only to belong to one of the categories
    to be considered a match by the request.
*/
QList<QPlaceCategory> QPlaceSearchRequest::categories() const
{
    Q_D(const QPlaceSearchRequest);
    return d->categories;
}

/*!
    Sets the search request to search by a single \a category

    \sa setCategories()
*/
void QPlaceSearchRequest::setCategory(const QPlaceCategory &category)
{
    Q_D(QPlaceSearchRequest);
    d->categories.clear();

    if (!category.categoryId().isEmpty())
        d->categories.append(category);
}

/*!
    Sets the search request to search from the list of given \a categories.
    Any places returned during the search will match at least one of the \a
    categories.

    \sa setCategory()
*/
void QPlaceSearchRequest::setCategories(const QList<QPlaceCategory> &categories)
{
    Q_D(QPlaceSearchRequest);
    d->categories = categories;
}

/*!
    Returns search area.  The default search area is a null pointer.
*/
QGeoBoundingArea *QPlaceSearchRequest::searchArea() const
{
    Q_D(const QPlaceSearchRequest);
    return d->searchArea;
}

/*!
    Sets the search request to search within the given \a area.  Ownership of the \a area is
    transferred to the request,  who is now responsible for pointer deletion.  If a new \a area
    is being assigned, the old area is deleted.
*/
void QPlaceSearchRequest::setSearchArea(QGeoBoundingArea *area)
{
    Q_D(QPlaceSearchRequest);
    if (d->searchArea != area)
        delete d->searchArea;

    d->searchArea = area;
}

/*!
    Returns the maximum number of search term corrections that may be returned.
*/
int QPlaceSearchRequest::maximumCorrections() const
{
    Q_D(const QPlaceSearchRequest);
    return d->dymNumber;
}

/*!
    Sets maximum \a number of search term corrections that may be returned.
*/
void QPlaceSearchRequest::setMaximumCorrections(int number)
{
    Q_D(QPlaceSearchRequest);
    d->dymNumber = number;
}

/*!
    Returns the visibility scope used when searching for places.  The default value is
    QtLocation::UnspecifiedVisibility meaning that no explicit scope has been assigned.
    Places of any scope may be returned during the search.
*/
QtLocation::VisibilityScope QPlaceSearchRequest::visibilityScope() const
{
    Q_D(const QPlaceSearchRequest);
    return d->visibilityScope;
}

/*!
    Sets the visibiliy \a scope used when searching for places.
*/
void QPlaceSearchRequest::setVisibilityScope(QtLocation::VisibilityScope scope)
{
    Q_D(QPlaceSearchRequest);
    d->visibilityScope = scope;
}

/*!
    Returns the relevance hint of the request.  The hint is given to the provider
    to help but not dictate the ranking of results.  eg providng a distance hint
    may give closer places a higher ranking but it doesn't necessarily mean
    that he results will be ordered strictly according to distance.
*/
QPlaceSearchRequest::RelevanceHint QPlaceSearchRequest::relevanceHint() const
{
    Q_D(const QPlaceSearchRequest);
    return d->relevanceHint;
}

/*!
    Sets the relevance \a hint to be used when searching for a place.
*/
void QPlaceSearchRequest::setRelevanceHint(QPlaceSearchRequest::RelevanceHint hint)
{
    Q_D(QPlaceSearchRequest);
    d->relevanceHint = hint;
}

/*!
    Returns the maximum number of search results to retrieve.

    A negative value for limit means that it is undefined.  It is left up to the backend
    provider to choose an appropriate number of results to return.  The default limit is -1.
*/
int QPlaceSearchRequest::limit() const
{
    Q_D(const QPlaceSearchRequest);
    return d->limit;
}

/*!
    Set the maximum number of search results to retrieve to \a limit.
*/
void QPlaceSearchRequest::setLimit(int limit)
{
    Q_D(QPlaceSearchRequest);
    d->limit = limit;
}

/*!
    Returns the offset index of the first item that is to be retrieved.

    The default offset is 0.
*/
int QPlaceSearchRequest::offset() const
{
    Q_D(const QPlaceSearchRequest);
    return d->offset;
}

/*!
    Sets the starting index of the first item to be retrieved
    to \a offset.
*/
void QPlaceSearchRequest::setOffset(int offset)
{
    Q_D(QPlaceSearchRequest);
    d->offset = offset;
}

/*!
    Clears the search request.
*/
void QPlaceSearchRequest::clear()
{
    Q_D(QPlaceSearchRequest);
    d->clear();
}

inline QPlaceSearchRequestPrivate* QPlaceSearchRequest::d_func()
{
    return static_cast<QPlaceSearchRequestPrivate *>(d_ptr.data());
}

inline const QPlaceSearchRequestPrivate* QPlaceSearchRequest::d_func() const
{
    return static_cast<const QPlaceSearchRequestPrivate *>(d_ptr.constData());
}

QT_END_NAMESPACE
