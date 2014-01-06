/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "rewritertransaction.h"
#include <abstractview.h>
#include <designdocument.h>
#include <qmldesignerplugin.h>

namespace QmlDesigner {



QList<QByteArray> RewriterTransaction::m_identifierList;
bool RewriterTransaction::m_activeIdentifier = !qgetenv("QML_DESIGNER_TRACE_REWRITER_TRANSACTION").isEmpty();

RewriterTransaction::RewriterTransaction() : m_valid(false)
{
}

RewriterTransaction::RewriterTransaction(AbstractView *_view, const QByteArray &identifier)
    : m_view(_view),
      m_identifier(identifier),
      m_valid(true)
{
    Q_ASSERT(view());

    static int identifierNumber = 0;
    m_identifierNumber = identifierNumber++;

    if (m_activeIdentifier) {
        qDebug() << "Begin RewriterTransaction:" << m_identifier << m_identifierNumber;
        m_identifierList.append(m_identifier + QByteArrayLiteral("-") + QByteArray::number(m_identifierNumber));
    }

    view()->emitRewriterBeginTransaction();
}

RewriterTransaction::~RewriterTransaction()
{
    commit();
}

bool RewriterTransaction::isValid() const
{
    return m_valid;
}

void RewriterTransaction::commit()
{
    if (m_valid) {
        m_valid = false;
        view()->emitRewriterEndTransaction();

        if (m_activeIdentifier) {
            qDebug() << "Commit RewriterTransaction:" << m_identifier << m_identifierNumber;
            bool success = m_identifierList.removeOne(m_identifier + QByteArrayLiteral("-") + QByteArray::number(m_identifierNumber));
            Q_ASSERT(success);
        }
    }
}

void RewriterTransaction::rollback()
{
    // TODO: should be implemented with a function in the rewriter
    if (m_valid) {
        m_valid = false;
        view()->emitRewriterEndTransaction();
        QmlDesignerPlugin::instance()->currentDesignDocument()->undo();

        if (m_activeIdentifier) {
            qDebug() << "Rollback RewriterTransaction:" << m_identifier << m_identifierNumber;
            m_identifierList.removeOne(m_identifier + QByteArrayLiteral("-") + QByteArray::number(m_identifierNumber));
       }
    }
}

AbstractView *RewriterTransaction::view()
{
    return m_view.data();
}

RewriterTransaction::RewriterTransaction(const RewriterTransaction &other)
        : m_valid(false)
{
    if (&other != this) {
        m_valid = other.m_valid;
        m_view = other.m_view;
        m_identifier = other.m_identifier;
        m_identifierNumber = other.m_identifierNumber;
        other.m_valid = false;
    }
}

RewriterTransaction& RewriterTransaction::operator=(const RewriterTransaction &other)
{
    if (!m_valid && (&other != this)) {
        m_valid = other.m_valid;
        m_view = other.m_view;
        m_identifier = other.m_identifier;
        m_identifierNumber = other.m_identifierNumber;
        other.m_valid = false;
    }

    return *this;
}

} //QmlDesigner

