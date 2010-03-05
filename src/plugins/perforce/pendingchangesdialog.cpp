/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include <QtCore/QRegExp>

#include "pendingchangesdialog.h"

using namespace Perforce::Internal;

PendingChangesDialog::PendingChangesDialog(const QString &data, QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    if (!data.isEmpty()) {
        QRegExp r("Change\\s(\\d+).*\\s\\*pending\\*\\s(.+)\n");
        r.setMinimal(true);
        int pos = 0;
        QListWidgetItem *item;
        while ((pos = r.indexIn(data, pos)) != -1) {
            item = new QListWidgetItem(tr("Change %1: %2").arg(r.cap(1))
                .arg(r.cap(2).trimmed()), m_ui.listWidget);
            item->setData(234, r.cap(1).trimmed());
            ++pos;
        }
    }
    m_ui.listWidget->setSelectionMode(QListWidget::SingleSelection);
    if (m_ui.listWidget->count()) {
        m_ui.listWidget->setCurrentRow(0);
        m_ui.submitButton->setEnabled(true);
    } else {
        m_ui.submitButton->setEnabled(false);
    }
}

int PendingChangesDialog::changeNumber() const
{
    QListWidgetItem *item = m_ui.listWidget->item(m_ui.listWidget->currentRow());
    if (!item)
        return -1;
    bool ok = true;
    int i = item->data(234).toInt(&ok);
    return ok ? i : -1;
}
