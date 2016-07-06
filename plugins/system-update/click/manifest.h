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

#ifndef CLICK_MANIFEST_H
#define CLICK_MANIFEST_H

#include <QObject>
#include <QJsonArray>

namespace UpdatePlugin
{
namespace Click
{
class Manifest : public QObject
{
    Q_OBJECT
public:
    Manifest(QObject *parent = 0) : QObject(parent) {};
    virtual ~Manifest() {};
    virtual void request() = 0;

signals:
    void requestSucceeded(const QJsonArray &manifest);
    void requestFailed();
};
} // Click
} // UpdatePlugin

#endif // CLICK_MANIFEST_H
