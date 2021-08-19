/*
 * Copyright (C) by Felix Weilbach <felix.weilbach@nextcloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "userstatusselectordialog.h"
#include "accountfwd.h"
#include "ocsuserstatusconnector.h"
#include "userstatusselectormodel.h"
#include "emojimodel.h"
#include "account.h"

#include <QQuickView>
#include <QLoggingCategory>
#include <QQmlError>
#include <QQmlEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QCloseEvent>
#include <qobject.h>
#include <qqmlcomponent.h>
#include <qqmlengine.h>
#include <qquickwindow.h>

Q_LOGGING_CATEGORY(lcUserStatusDialog, "nextcloud.gui.systray")

namespace OCC {
namespace UserStatusSelectorDialog {

    class CloseEventFilter : public QObject
    {
        Q_OBJECT
    public:
        CloseEventFilter(QObject *parent)
            : QObject(parent)
            , _parent(parent)
        {
        }

    protected:
        bool eventFilter(QObject *obj, QEvent *event)
        {
            if (event->type() == QEvent::Close) {
                _parent->deleteLater();
            }

            return QObject::eventFilter(obj, event);
        }

    private:
        QObject *_parent;
    };


    static bool logErrors(const QList<QQmlError> &errors)
    {
        bool isError = false;

        for (const auto &error : errors) {
            isError = true;
            qCWarning(lcUserStatusDialog) << error.toString();
        }

        return isError;
    }

    class UserStatusSelectorDialogView : public QObject
    {
        Q_OBJECT

    public:
        explicit UserStatusSelectorDialogView(std::shared_ptr<UserStatusConnector> userStatusConnector, QObject *parent = nullptr)
            : QObject(parent)
        {
            _engine = new QQmlEngine(this);
            _component = new QQmlComponent(_engine);
            _component->loadUrl(QUrl("qrc:/qml/src/gui/UserStatusSelectorDialog.qml"));
            logErrors(_component->errors());
            _model = new UserStatusSelectorModel(userStatusConnector, this);
            QObject::connect(_model, &UserStatusSelectorModel::finished, this, [this]() {
                deleteLater();
            });
            _window = qobject_cast<QQuickWindow *>(_component->createWithInitialProperties({ { "model", QVariant::fromValue(_model) } }));
            logErrors(_component->errors());
            auto closeEventFilter = new CloseEventFilter(this);
            _window->installEventFilter(closeEventFilter);
        }

        ~UserStatusSelectorDialogView()
        {
            _window->close();
            _window->deleteLater();
            _model->deleteLater();
            _component->deleteLater();
            _engine->deleteLater();
        }

    private:
        QQmlEngine *_engine;
        QQmlComponent *_component;
        QQuickWindow *_window;
        UserStatusSelectorModel *_model;
    };

    void show(AccountPtr account)
    {
        new UserStatusSelectorDialogView(account->userStatusConnector(), account.data());
    }
}
}

#include "userstatusselectordialog.moc"
