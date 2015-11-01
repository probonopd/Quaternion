/******************************************************************************
 * Copyright (C) 2015 Felix Rohrbach <kde@fxrh.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "basejob.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "../connectiondata.h"

using namespace QMatrixClient;

class BaseJob::Private
{
    public:
        Private(ConnectionData* c) : connection(c), reply(0) {}
        
        ConnectionData* connection;
        QNetworkReply* reply;
};

BaseJob::BaseJob(ConnectionData* connection)
    : d(new Private(connection))
{
}

BaseJob::~BaseJob()
{
    delete d;
}

ConnectionData* BaseJob::connection() const
{
    return d->connection;
}

QNetworkReply* BaseJob::get(const QString& path, const QUrlQuery& query) const
{
    QUrl url = d->connection->baseUrl();
    url.setPath( url.path() + "/" + path );
    url.setQuery(query);
    QNetworkRequest req = QNetworkRequest(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    d->reply = d->connection->nam()->get(req);
    connect( d->reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
             this, &BaseJob::networkError ); // http://doc.qt.io/qt-5/qnetworkreply.html#error-1
    return d->reply;
}

QNetworkReply* BaseJob::put(const QString& path, const QJsonDocument& data, const QUrlQuery& query) const
{
    QUrl url = d->connection->baseUrl();
    url.setPath( url.path() + "/" + path );
    url.setQuery(query);
    QNetworkRequest req = QNetworkRequest(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    d->reply = d->connection->nam()->put(req, data.toJson());
    connect( d->reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
             this, &BaseJob::networkError );
    return d->reply;
}

QNetworkReply* BaseJob::post(const QString& path, const QJsonDocument& data, const QUrlQuery& query) const
{
    QUrl url = d->connection->baseUrl();
    url.setPath( url.path() + "/" + path );
    url.setQuery(query);
    QNetworkRequest req = QNetworkRequest(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    d->reply = d->connection->nam()->post(req, data.toJson());
    connect( d->reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
             this, &BaseJob::networkError );
    return d->reply;
}

void BaseJob::fail(int errorCode, QString errorString)
{
    setError( errorCode );
    setErrorText( errorString );
    emitResult();
}

void BaseJob::networkError(QNetworkReply::NetworkError code)
{
    fail( KJob::UserDefinedError+1, d->reply->errorString() );
}