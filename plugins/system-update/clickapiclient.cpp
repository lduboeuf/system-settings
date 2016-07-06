/*
 * This file is part of system-settings
 *
 * Copyright (C) 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clickapiclient.h"
#include "helpers.h"
#include "systemupdate.h"

namespace UpdatePlugin
{
ClickApiClient::ClickApiClient(NetworkAccessManager *nam,
                               QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{
    initializeNam();
}

ClickApiClient::~ClickApiClient()
{
    cancel();
}

void ClickApiClient::initializeNam()
{
    connect(m_nam, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(requestFinished(QNetworkReply *)));
    connect(m_nam, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError>&)),
            this, SLOT(requestSslFailed(QNetworkReply *, const QList<QSslError>&)));
}

void ClickApiClient::getMetadata(const QUrl &url,
                                 const QByteArray &packageNames)
{
    // Create list of frameworks.
    std::stringstream frameworks;
    for (auto f : Helpers::getAvailableFrameworks()) {
        frameworks << "," << f;
    }

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("X-Ubuntu-Frameworks"),
            QByteArray::fromStdString(frameworks.str()));
    request.setRawHeader(QByteArray("X-Ubuntu-Architecture"),
            QByteArray::fromStdString(Helpers::getArchitecture()));
    request.setUrl(url);
    request.setOriginatingObject(this);

    initializeReply(m_nam->post(request, packageNames));
}

void ClickApiClient::getToken(const QUrl &url)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setOriginatingObject(this);

    initializeReply(m_nam->head(request));
}

void ClickApiClient::initializeReply(QNetworkReply *reply)
{
    qWarning() << "click api client: init reply" << reply;
    connect(this, SIGNAL(abortNetworking()), reply, SLOT(abort()));
}

void ClickApiClient::requestSslFailed(QNetworkReply *reply,
                                      const QList<QSslError> &errors)
{
    QString errorString = "SSL error: ";
    foreach (const QSslError &err, errors) {
        errorString += err.errorString();
    }
    qCritical() << errorString;
    Q_EMIT serverError();
    reply->deleteLater();
}

void ClickApiClient::requestFinished(QNetworkReply *reply)
{
    // TODO: refactor this bit
    if (reply->request().originatingObject() == this) {
        qWarning() << "click api client: something finished" << reply << "and we will handle it.";
    } else {
        qWarning() << "click api client: something finished" << reply << "but we will NOT handle it.";
        return;
    }

    if (!validReply(reply)) {
        // Error signals are already sent.
        reply->deleteLater();
        return;
    }
    qWarning() << "valid reply!";

    switch (reply->error()) {
    case QNetworkReply::NoError:
        requestSucceeded(reply);
        return;
    case QNetworkReply::TemporaryNetworkFailureError:
    case QNetworkReply::UnknownNetworkError:
    case QNetworkReply::UnknownProxyError:
    case QNetworkReply::UnknownServerError:
        Q_EMIT networkError();
        break;
    default:
        Q_EMIT serverError();
    }

    reply->deleteLater();
}

void ClickApiClient::requestSucceeded(QNetworkReply *reply)
{
    qWarning() << Q_FUNC_INFO << reply->request().url().toString();
    QByteArray content(reply->readAll());
    if (reply->hasRawHeader(X_CLICK_TOKEN)) {
        qWarning() << Q_FUNC_INFO << "had header" << X_CLICK_TOKEN << reply->rawHeader(X_CLICK_TOKEN);
        QString header(reply->rawHeader(X_CLICK_TOKEN));
        // qWarning() << "setting click token to" << header;
        // m_update->setToken(header);
        Q_EMIT tokenRequestSucceeded(header);
    } else if (!content.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "had metadata";
        Q_EMIT metadataRequestSucceeded(content);
    } else {
        qWarning() << Q_FUNC_INFO << "not understood";
    }

    reply->deleteLater();
}

bool ClickApiClient::validReply(const QNetworkReply *reply)
{
    auto statusAttr = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusAttr.isValid()) {
        Q_EMIT networkError();
        qCritical() << "Could not parse status code.";
        return false;
    }

    int httpStatus = statusAttr.toInt();
    qWarning() << "click api client: HTTP Status: " << httpStatus;

    if (httpStatus == 401 || httpStatus == 403) {
        qCritical() << QString("Server responded with %1.").arg(httpStatus);
        Q_EMIT credentialError();
        return false;
    }

    if (httpStatus == 404) {
        qCritical() << "Server responded with 404.";
        Q_EMIT serverError();
        return false;
    }

    return true;
}

// void ClickApiClient::handleTokenReply(QNetworkReply *reply)
// {

// }

// void ClickApiClient::handleMetadataReply(QNetworkReply *reply)
// {

// }

void ClickApiClient::cancel()
{
    // Tell each reply to abort. See initializeReply().
    Q_EMIT abortNetworking();
}

} // UpdatePlugin
