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

#include "helpers.h"
#include "sso_impl.h"

namespace UpdatePlugin
{
namespace Click
{
SSOImpl::SSOImpl(QObject *parent)
    : SSO(parent)
    , m_service(new UbuntuOne::SSOService(this))
{
    connect(m_service, SIGNAL(credentialsFound(const UbuntuOne::Token&)),
            this, SLOT(handleCredentialsFound(const UbuntuOne::Token&)));
    connect(m_service, SIGNAL(credentialsNotFound()),
            this, SLOT(handleCredentialsFailed()));
    connect(m_service, SIGNAL(credentialsDeleted()),
            this, SLOT(handleCredentialsFailed()));
}

void SSOImpl::requestCredentials()
{
    m_service->getCredentials();
}

void SSOImpl::invalidateCredentials()
{
    m_service->invalidateCredentials();
}

void SSOImpl::handleCredentialsFound(const UbuntuOne::Token &token)
{
    Q_EMIT credentialsRequestSucceeded(token);
}

void SSOImpl::handleCredentialsFailed()
{
    Q_EMIT credentialsRequestFailed();
}

} // Click
} // UpdatePlugin
