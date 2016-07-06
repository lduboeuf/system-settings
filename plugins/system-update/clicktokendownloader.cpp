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

#include "clicktokendownloader.h"
#include "helpers.h"
#include "networkaccessmanager.h"

namespace UpdatePlugin
{
ClickTokenDownloader::ClickTokenDownloader(QObject *parent,
                                           Update *update)
    : QObject(parent)
    , m_update(update)
    , m_client(SystemUpdate::instance()->nam(), this)
    , m_authToken(UbuntuOne::Token())
{
    init();
    qWarning() << "click token download init for" << update->identifier();
}

ClickTokenDownloader::~ClickTokenDownloader()
{
    cancel();
}

void ClickTokenDownloader::init()
{
    // connect(&m_client, SIGNAL(success(QNetworkReply*)),
    //         this, SLOT(handleSuccess(QNetworkReply*)));
    connect(&m_client, SIGNAL(tokenRequestSucceeded(const QString)),
            this, SLOT(handleSuccess(const QString)));
    connect(&m_client, SIGNAL(networkError()),
            this, SLOT(handleFailure()));
    connect(&m_client, SIGNAL(serverError()),
            this, SLOT(handleFailure()));
    connect(&m_client, SIGNAL(credentialError()),
            this, SLOT(handleFailure()));
}

void ClickTokenDownloader::setAuthToken(const UbuntuOne::Token &authToken)
{
    m_authToken = authToken;
}

void ClickTokenDownloader::cancel()
{
    m_client.cancel();
}

void ClickTokenDownloader::requestToken()
{
    qWarning() << "requests token on url" << m_update->identifier();
    if (!m_authToken.isValid() && !Helpers::isIgnoringCredentials()) {
        qWarning() << "token invalid";
        Q_EMIT tokenRequestFailed(m_update);
        return;
    }

    QString authHeader = m_authToken.signUrl(
        m_update->downloadUrl(), QStringLiteral("HEAD"), true
    );

    if (authHeader.isEmpty()) {
        // Already logged.
        tokenRequestFailed(m_update);
        return;
    }

    QString signUrl = Helpers::clickTokenUrl(m_update->downloadUrl());
    QUrl query(signUrl);
    query.setQuery(authHeader);
    m_client.getToken(query);
}

void ClickTokenDownloader::handleSuccess(const QString &token)
{
    m_update->setToken(token);
    if (token.isEmpty()) {
        tokenRequestFailed(m_update);
    } else {
        tokenRequestSucceeded(m_update);
    }
}

void ClickTokenDownloader::handleFailure()
{
    Q_EMIT tokenRequestFailed(m_update);
}
}// UpdatePlugin
