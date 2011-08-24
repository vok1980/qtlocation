/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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
****************************************************************************/

#include "qdeclarativegeocodemodel_p.h"

#include "qdeclarativegeolocation_p.h"
#include <QtDeclarative/qdeclarativeinfo.h>

#include <qgeoserviceprovider.h>
#include <qgeocodingmanager.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass GeocodeModel

    \brief The GeocodeModel element provides support for searching operations related
           to geographic information.
    \ingroup qml-geocoding
    \since 5.0

    The GeocodeModel is a model used to perform geocoding. This includes both geocoding
    (address to coordinate) and reverse geocoding (coordinate to address).
    The geocoding result provider is determined by the \l plugin. The geocoding data is set
    in \l query.

    The model provides a single data role, the "locationData" role which
    is a \l Location element.
*/

QDeclarativeGeocodeModel::QDeclarativeGeocodeModel(QObject* parent)
    : QAbstractListModel(parent),
      autoUpdate_(false),
      complete_(false),
      reply_(0),
      plugin_(0),
      boundingArea_(0),
      status_(QDeclarativeGeocodeModel::Null),
      error_(QDeclarativeGeocodeModel::NoError),
      coordinate_(0),
      address_(0),
      limit_(-1),
      offset_(0)
{
    QHash<int, QByteArray> roleNames;
    roleNames = QAbstractItemModel::roleNames();
    roleNames.insert(LocationRole, "locationData");
    setRoleNames(roleNames);
}

QDeclarativeGeocodeModel::~QDeclarativeGeocodeModel()
{
    qDeleteAll(declarativeLocations_);
    declarativeLocations_.clear();
    delete reply_;
}

// From QDeclarativeParserStatus
void QDeclarativeGeocodeModel::componentComplete()
{
    complete_ = true;
    if (autoUpdate_)
        update();
}

QGeoBoundingArea* QDeclarativeGeocodeModel::boundingArea()
{
    if (qobject_cast<QDeclarativeGeoBoundingBox*>(boundingArea_) && boundingBox_.isValid()) {
        return &boundingBox_;
    } else if (qobject_cast<QDeclarativeGeoBoundingCircle*>(boundingArea_) && boundingCircle_.isValid()) {
        return &boundingCircle_;
    }
    return 0;
}

void QDeclarativeGeocodeModel::update()
{
    if (!complete_)
        return;

    if (!plugin_) {
        qmlInfo(this) << tr("Cannot geocode, plugin not set.");
        return;
    }

    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    if (!serviceProvider)
        return;

    QGeocodingManager *geocodingManager = serviceProvider->geocodingManager();
    if (!geocodingManager) {
        qmlInfo(this) << tr("Cannot geocode, search manager (/plugin) not set.");
        return;
    }
    if ((!coordinate_ || !coordinate_->coordinate().isValid()) &&
            (!address_ || address_->address().isEmpty()) &&
            (searchString_.isEmpty())) {
        qmlInfo(this) << tr("Cannot geocode, valid query not set.");
        return;
    }
    abortRequest(); // abort possible previous requests
    setErrorString("");   // clear previous error string
    setError(NoError);

    if (coordinate_) {
        setStatus(QDeclarativeGeocodeModel::Loading);
        reply_ = geocodingManager->reverseGeocode(coordinate_->coordinate(), boundingArea());
        if (reply_->isFinished()) {
            if (reply_->error() == QGeocodeReply::NoError) {
                geocodeFinished(reply_);
            } else {
                geocodeError(reply_, reply_->error(), reply_->errorString());
            }
        }
    } else if (address_) {
        setStatus(QDeclarativeGeocodeModel::Loading);
        reply_ = geocodingManager->geocode(address_->address(), boundingArea());
        if (reply_->isFinished()) {
            if (reply_->error() == QGeocodeReply::NoError) {
                geocodeFinished(reply_);
            } else {
                geocodeError(reply_, reply_->error(), reply_->errorString());
            }
        }
    } else if (!searchString_.isEmpty()) {
        setStatus(QDeclarativeGeocodeModel::Loading);
        reply_ = geocodingManager->geocode(searchString_, limit_, offset_, boundingArea());
        if (reply_->isFinished()) {
            if (reply_->error() == QGeocodeReply::NoError) {
                geocodeFinished(reply_);
            } else {
                geocodeError(reply_, reply_->error(), reply_->errorString());
            }
        }
    }
}

void QDeclarativeGeocodeModel::abortRequest()
{
    if (reply_) {
        reply_->abort();
        reply_->deleteLater();
        reply_ = 0;
    }
}

void QDeclarativeGeocodeModel::queryContentChanged()
{
    if (autoUpdate_)
        update();
}

// From QAbstractListModel
int QDeclarativeGeocodeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return declarativeLocations_.count();
}

QVariant QDeclarativeGeocodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= declarativeLocations_.count())
        return QVariant();
    if (role == QDeclarativeGeocodeModel::LocationRole) {
        QObject* locationObject = declarativeLocations_.at(index.row());
        Q_ASSERT(locationObject);
        return QVariant::fromValue(locationObject);
    }
    return QVariant();
}

void QDeclarativeGeocodeModel::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (plugin_ == plugin)
        return;
    reset(); // reset the model
    plugin_ = plugin;
    if (complete_)
        emit pluginChanged();
    QGeoServiceProvider *serviceProvider = plugin_->sharedGeoServiceProvider();
    QGeocodingManager *geocodingManager = serviceProvider->geocodingManager();
    if (!geocodingManager || serviceProvider->error() != QGeoServiceProvider::NoError) {
        qmlInfo(this) << tr("Warning: Plugin does not support (reverse) geocoding.");
        return;
    }
    connect(geocodingManager, SIGNAL(finished(QGeocodeReply*)),
            this, SLOT(geocodeFinished(QGeocodeReply*)));
    connect(geocodingManager, SIGNAL(error(QGeocodeReply*,QGeocodeReply::Error,QString)),
            this, SLOT(geocodeError(QGeocodeReply*,QGeocodeReply::Error,QString)));
}

/*!
    \qmlproperty Plugin GeocodeModel::plugin

    This property holds the plugin that providers the actual geocoding service.
    Note that all plugins do not necessarily provide routing (could e.g. provide
    only routing or maps).

    \sa Plugin
*/

QDeclarativeGeoServiceProvider* QDeclarativeGeocodeModel::plugin() const
{
    return plugin_;
}

void QDeclarativeGeocodeModel::setBounds(QObject* bounds)
{
    if (boundingArea_ == bounds)
            return;
    if (qobject_cast<QDeclarativeGeoBoundingBox*>(bounds)) {
        boundingBox_ = qobject_cast<QDeclarativeGeoBoundingBox*>(bounds)->box();
    } else if (qobject_cast<QDeclarativeGeoBoundingCircle*>(bounds)) {
        boundingCircle_ = qobject_cast<QDeclarativeGeoBoundingCircle*>(bounds)->circle();
    } else {
        qmlInfo(this) << tr("Unsupported bound type (Box and Circle supported)");
        return;
    }
    boundingArea_ = bounds;
    emit boundsChanged();
}

/*!
    \qmlproperty bounding area GeocodeModel::bounds

    This property holds the bounding area used to limit the results to those
    within the area. his is particularly useful if query is only partially filled out,
    as the service will attempt to (reverse) geocode all matches for the specified data.

    Accepted element types are \l BoundingBox and \l BoundingCircle.

*/

QObject* QDeclarativeGeocodeModel::bounds() const
{
    return boundingArea_;
}

void QDeclarativeGeocodeModel::geocodeFinished(QGeocodeReply *reply)
{
    if (reply->error() != QGeocodeReply::NoError) {
        return;
    }
    int oldCount = declarativeLocations_.count();
    setLocations(reply->locations());
    setErrorString("");
    setError(NoError);
    setStatus(QDeclarativeGeocodeModel::Ready);
    reply->deleteLater();
    reply_ = 0;
    emit locationsChanged();
    if (oldCount != declarativeLocations_.count())
        emit countChanged();
}

void QDeclarativeGeocodeModel::geocodeError(QGeocodeReply *reply,
        QGeocodeReply::Error error,
        const QString &errorString)
{
    Q_UNUSED(error);
    int oldCount = declarativeLocations_.count();
    if (oldCount > 0) {
        // Reset the model
        setLocations(reply->locations());
        emit locationsChanged();
        emit countChanged();
    }
    setErrorString(errorString);
    setError(static_cast<QDeclarativeGeocodeModel::GeocodeError>(error));
    setStatus(QDeclarativeGeocodeModel::Error);
    reply->deleteLater();
    reply_ = 0;
}

/*!
    \qmlproperty enumeration GeocodeModel::status

    This read-only property holds the current status of the model.

    \list
    \o GeocodeModel.Null - No geocode requests have been issued or \l reset has been called.
    \o GeocodeModel.Ready - Geocode request(s) have finished successfully.
    \o GeocodeModel.Loading - Geocode request has been issued but not yet finished
    \o GeocodeModel.Error - Geocoding error has occured, details are in \l error and \l errorString
    \endlist
*/

QDeclarativeGeocodeModel::Status QDeclarativeGeocodeModel::status() const
{
    return status_;
}

void QDeclarativeGeocodeModel::setStatus(QDeclarativeGeocodeModel::Status status)
{
    if (status_ == status)
        return;
    status_ = status;
    emit statusChanged();
}

/*!
    \qmlproperty enumeration GeocodeModel::error

    This read-only property holds the latest error value of the geocoding request.

    \list
    \o GeocodeModel.NoError - No error has occurred
    \o GeocodeModel.EngineNotSetError - The plugin/service provider used does not support (reverse) geocoding
    \o GeocodeModel.CommunicationError - An error occurred while communicating with the service provider
    \o GeocodeModel.ParseError - The response from the service provider was in an unrecognizable format
    \o GeocodeModel.UnsupportedOptionError - The requested operation or one of the options for the operation are not supported by the service provider.
    \o GeocodeModel.CombinationError - An error occurred while results where being combined from multiple sources
    \o GeocodeModel.UnknownError - An error occurred which does not fit into any of the other categories
    \endlist
*/

QDeclarativeGeocodeModel::GeocodeError QDeclarativeGeocodeModel::error() const
{
    return error_;
}

void QDeclarativeGeocodeModel::setError(GeocodeError error)
{
    if (error_ == error)
        return;
    error_ = error;
    emit errorChanged();
}

/*!
    \qmlproperty string GeocodeModel::errorString

    This read-only property holds the textual presentation of latest geocoding error.
    If no error has occured or the model has been reset, an empty string is returned.

    An empty string may also be returned if an error occurred which has no associated
    textual representation.
*/

QString QDeclarativeGeocodeModel::errorString() const
{
    return errorString_;
}

void QDeclarativeGeocodeModel::setErrorString(const QString &error)
{
    if (errorString_ == error)
        return;
    errorString_ = error;
    emit errorStringChanged();
}

void QDeclarativeGeocodeModel::setLocations(const QList<QGeoLocation> &locations)
{
    beginResetModel();
    qDeleteAll(declarativeLocations_);
    declarativeLocations_.clear();
    for (int i = 0;  i < locations.count(); ++i) {
        QDeclarativeGeoLocation* location = new QDeclarativeGeoLocation(locations.at(i), this);
        declarativeLocations_.append(location);
    }
    endResetModel();
}

/*!
    \qmlproperty int GeocodeModel::count

    This property holds how many locations the model currently has
    Amongst other uses, you can use this value when accessing locations
    via the GeocodeModel::get -method.
*/

int QDeclarativeGeocodeModel::count() const
{
    return declarativeLocations_.count();
}

/*!
    \qmlmethod GeocodeModel::get(int)

    Returns the Location at given index. Use \l count property to check the
    amount of locations available. The locations are indexed from zero, so the accessible range
    is 0...(count - 1).

    If you access out of bounds, a zero (null object) is returned and a warning is issued.
*/

QDeclarativeGeoLocation* QDeclarativeGeocodeModel::get(int index)
{
    if (index < 0 || index >= declarativeLocations_.count()) {
        qmlInfo(this) << tr("Error, too big or small index in get(): ") << index;
        return 0;
    }
    return declarativeLocations_.at(index);
}

/*!
    \qmlproperty int GeocodeModel::limit

    This property holds the maximum number of results. The \l limit and \l offset values only
    applicable with free string geocoding (i.e. they are not considered when using addresses
    or coordinates in the search query).

    If limit is -1 the entire result set will be returned, otherwise at most limit results will be returned.
    The limit and \l offset results can be used together to implement paging.
*/

int QDeclarativeGeocodeModel::limit() const
{
    return limit_;
}

void QDeclarativeGeocodeModel::setLimit(int limit)
{
    if (limit == limit_)
        return;
    limit_ = limit;
    if (autoUpdate_) {
        update();
    }
    emit limitChanged();
}

/*!
    \qmlproperty int GeocodeModel::offset

    This property tells not to return the first 'offset' number of the results. The \l limit
    and \l offset values are only applicable with free string geocoding (i.e. they are not considered
    when using addresses or coordinates in the search query).

    The \l limit and offset results can be used together to implement paging.
*/

int QDeclarativeGeocodeModel::offset() const
{
    return offset_;
}

void QDeclarativeGeocodeModel::setOffset(int offset)
{
    if (offset == offset_)
        return;
    offset_ = offset;
    if (autoUpdate_) {
        update();
    }
    emit offsetChanged();
}

/*!
    \qmlmethod GeocodeModel::clear()

    Clears the location data of the model. Any outstanding requests or
    errors remain intact.
*/

void QDeclarativeGeocodeModel::clear()
{
    bool hasChanged = !declarativeLocations_.isEmpty();
    setLocations(QList<QGeoLocation>());
    if (hasChanged)
        emit countChanged();
}

/*!
    \qmlmethod GeocodeModel::reset()

    Resets the model. All location data is cleared, any outstanding requests
    are aborted and possible errors are cleared. Model status will be set
    to GeocodeModel.Null
*/

void QDeclarativeGeocodeModel::reset()
{
    clear();
    abortRequest();
    setErrorString("");
    setError(NoError);
    setStatus(QDeclarativeGeocodeModel::Null);
}

/*!
    \qmlproperty QVariant GeocodeModel::query

    This property holds the data of the geocoding request.
    The property accepts three types of queries, which determines both the data and
    the type of action to be performed:

    \list
    \o Address - Geocoding (address to coordinate)
    \o Coordinate - Reverse geocoding (coordinate to address)
    \o string - Geocoding (address to coordinate)
    \endlist
*/

QVariant QDeclarativeGeocodeModel::query() const
{
    return queryVariant_;
}

void QDeclarativeGeocodeModel::setQuery(const QVariant& query)
{
    if (query == queryVariant_)
        return;
    QObject* object = qvariant_cast<QObject*>(query);
    if (qobject_cast<QDeclarativeCoordinate*>(object)) {
        if (coordinate_)
            coordinate_->disconnect(this);
        if (address_)
            address_->disconnect(this);
        coordinate_ = qobject_cast<QDeclarativeCoordinate*>(object);
        connect(coordinate_, SIGNAL(latitudeChanged(double)), this, SLOT(queryContentChanged()));
        connect(coordinate_, SIGNAL(longitudeChanged(double)), this, SLOT(queryContentChanged()));
        connect(coordinate_, SIGNAL(altitudeChanged(double)), this, SLOT(queryContentChanged()));
        address_ = 0;
        searchString_.clear();
    } else if (qobject_cast<QDeclarativeGeoAddress*>(object)) {
        if (address_)
            address_->disconnect(this);
        if (coordinate_)
            coordinate_->disconnect(this);
        address_ = qobject_cast<QDeclarativeGeoAddress*>(object);
        connect(address_, SIGNAL(countryChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(countryCodeChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(stateChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(countyChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(cityChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(districtChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(streetChanged()), this, SLOT(queryContentChanged()));
        connect(address_, SIGNAL(postcodeChanged()), this, SLOT(queryContentChanged()));
        coordinate_ = 0;
        searchString_.clear();
    } else if (query.type() == QVariant::String) {
        searchString_ = query.toString();
        if (address_)
            address_->disconnect(this);
        if (coordinate_)
            coordinate_->disconnect(this);
        address_ = 0;
        coordinate_ = 0;
    } else {
        qmlInfo(this) << tr("Unsupported query type for geocode model (Coordinate, string and Address supported).");
        return;
    }
    queryVariant_ = query;
    emit queryChanged();
    if (autoUpdate_)
        update();
}

/*!
    \qmlproperty bool GeocodeModel::autoUpdate

    This property instructs how the model should react on query changes -
    should it automatically update the model or do nothing.

    Caution: If setting this value to 'true', take also care that your application
    does not accidentally trigger huge amounts of unnecessary geocoding requests.
    In another words, be aware where in your application the query might change.
*/

bool QDeclarativeGeocodeModel::autoUpdate() const
{
    return autoUpdate_;
}

void QDeclarativeGeocodeModel::setAutoUpdate(bool update)
{
    if (autoUpdate_ == update)
        return;
    autoUpdate_ = update;
    emit autoUpdateChanged();
}

#include "moc_qdeclarativegeocodemodel_p.cpp"

QT_END_NAMESPACE
