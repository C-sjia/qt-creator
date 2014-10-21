/****************************************************************************
**
** Copyright (C) 2014 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the Qt Creator Enterprise Auto Test Add-on.
**
** Licensees holding valid Qt Enterprise licenses may use this file in
** accordance with the Qt Enterprise License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
**
****************************************************************************/

#include "testresultspane.h"
#include "testresultmodel.h"
#include "testresultdelegate.h"
#include "testrunner.h"
#include "testtreemodel.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>

#include <texteditor/texteditor.h>

#include <utils/itemviews.h>

#include <QDebug>
#include <QToolButton>

namespace Autotest {
namespace Internal {

TestResultsPane::TestResultsPane(QObject *parent) :
    Core::IOutputPane(parent),
    m_context(new Core::IContext(this))
{
    m_listView = new Utils::ListView;
    m_model = new TestResultModel(this);
    m_filterModel = new TestResultFilterModel(m_model, this);
    m_filterModel->setDynamicSortFilter(true);
    m_listView->setModel(m_filterModel);
    TestResultDelegate *trd = new TestResultDelegate(this);
    m_listView->setItemDelegate(trd);

    createToolButtons();

    connect(m_listView, &Utils::ListView::activated, this, &TestResultsPane::onItemActivated);
    connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
            trd, &TestResultDelegate::currentChanged);
}

void TestResultsPane::createToolButtons()
{
    m_runAll = new QToolButton(m_listView);
    m_runAll->setIcon(QIcon(QLatin1String(":/images/run.png")));
    m_runAll->setToolTip(tr("Run All Tests"));
    connect(m_runAll, &QToolButton::clicked, this, &TestResultsPane::onRunAllTriggered);

    m_runSelected = new QToolButton(m_listView);
    m_runSelected->setIcon(QIcon(QLatin1String(":/images/runselected.png")));
    m_runSelected->setToolTip(tr("Run Selected Tests"));
    connect(m_runSelected, &QToolButton::clicked, this, &TestResultsPane::onRunSelectedTriggered);

    m_filterButton = new QToolButton(m_listView);
    m_filterButton->setIcon(QIcon(QLatin1String(Core::Constants::ICON_FILTER)));
    m_filterButton->setToolTip(tr("Filter Test Results"));
    m_filterButton->setProperty("noArrow", true);
    m_filterButton->setAutoRaise(true);
    m_filterButton->setPopupMode(QToolButton::InstantPopup);
    m_filterMenu = new QMenu(m_filterButton);
    initializeFilterMenu();
    connect(m_filterMenu, &QMenu::triggered, this, &TestResultsPane::filterMenuTriggered);
    m_filterButton->setMenu(m_filterMenu);
}

static TestResultsPane *m_instance = 0;

TestResultsPane *TestResultsPane::instance()
{
    if (!m_instance)
        m_instance = new TestResultsPane;
    return m_instance;
}

TestResultsPane::~TestResultsPane()
{
    delete m_listView;
    m_instance = 0;
}

void TestResultsPane::addTestResult(const TestResult &result)
{
    m_model->addTestResult(result);
    if (!m_listView->isVisible())
        popup(Core::IOutputPane::NoModeSwitch);
    flash();
    navigateStateChanged();
}

QWidget *TestResultsPane::outputWidget(QWidget *parent)
{
    if (m_listView) {
        m_listView->setParent(parent);
    } else {
        m_listView = new Utils::ListView(parent);
    }
    return m_listView;
}

QList<QWidget *> TestResultsPane::toolBarWidgets() const
{
    return QList<QWidget *>() << m_runAll << m_runSelected << m_filterButton;
}

QString TestResultsPane::displayName() const
{
    return tr("Test Results");
}

int TestResultsPane::priorityInStatusBar() const
{
    return -666;
}

void TestResultsPane::clearContents()
{
    m_filterModel->clearTestResults();
    navigateStateChanged();
}

void TestResultsPane::visibilityChanged(bool)
{
}

void TestResultsPane::setFocus()
{
}

bool TestResultsPane::hasFocus() const
{
    return m_listView->hasFocus();
}

bool TestResultsPane::canFocus() const
{
    return true;
}

bool TestResultsPane::canNavigate() const
{
    return true;
}

bool TestResultsPane::canNext() const
{
    return m_filterModel->hasResults();
}

bool TestResultsPane::canPrevious() const
{
    return m_filterModel->hasResults();
}

void TestResultsPane::goToNext()
{
    if (!canNext())
        return;

    QModelIndex currentIndex = m_listView->currentIndex();
    if (currentIndex.isValid()) {
        int row = currentIndex.row() + 1;
        if (row == m_filterModel->rowCount(QModelIndex()))
            row = 0;
        currentIndex = m_filterModel->index(row, 0, QModelIndex());
    } else {
        currentIndex = m_filterModel->index(0, 0, QModelIndex());
    }
    m_listView->setCurrentIndex(currentIndex);
    onItemActivated(currentIndex);
}

void TestResultsPane::goToPrev()
{
    if (!canPrevious())
        return;

    QModelIndex currentIndex = m_listView->currentIndex();
    if (currentIndex.isValid()) {
        int row = currentIndex.row() - 1;
        if (row < 0)
            row = m_filterModel->rowCount(QModelIndex()) - 1;
        currentIndex = m_filterModel->index(row, 0, QModelIndex());
    } else {
        currentIndex = m_filterModel->index(m_filterModel->rowCount(QModelIndex()) - 1, 0, QModelIndex());
    }
    m_listView->setCurrentIndex(currentIndex);
    onItemActivated(currentIndex);
}

void TestResultsPane::onItemActivated(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    TestResult tr = m_filterModel->testResult(index);
    if (!tr.fileName().isEmpty())
        Core::EditorManager::openEditorAt(tr.fileName(), tr.line(), 0);
}

void TestResultsPane::onRunAllTriggered()
{
    TestRunner *runner = TestRunner::instance();
    runner->setSelectedTests(TestTreeModel::instance()->getAllTestCases());
    runner->runTests();
}

void TestResultsPane::onRunSelectedTriggered()
{
    TestRunner *runner = TestRunner::instance();
    runner->setSelectedTests(TestTreeModel::instance()->getSelectedTests());
    runner->runTests();
}

void TestResultsPane::initializeFilterMenu()
{
    QMap<ResultType, QString> textAndType;
    textAndType.clear();
    textAndType.insert(ResultType::PASS, QLatin1String("Pass"));
    textAndType.insert(ResultType::FAIL, QLatin1String("Fail"));
    textAndType.insert(ResultType::EXPECTED_FAIL, QLatin1String("Expected Fail"));
    textAndType.insert(ResultType::UNEXPECTED_PASS, QLatin1String("Unexpected Pass"));
    textAndType.insert(ResultType::SKIP, QLatin1String("Skip"));
    textAndType.insert(ResultType::MESSAGE_DEBUG, QLatin1String("Debug Messages"));
    textAndType.insert(ResultType::MESSAGE_WARN, QLatin1String("Warning Messages"));
    textAndType.insert(ResultType::MESSAGE_INTERNAL, QLatin1String("Internal Messages"));
    foreach (ResultType result, textAndType.keys()) {
        QAction *action = new QAction(m_filterMenu);
        action->setText(textAndType.value(result));
        action->setCheckable(true);
        action->setChecked(true);
        action->setData(result);
        m_filterMenu->addAction(action);
    }
}

void TestResultsPane::filterMenuTriggered(QAction *action)
{
    m_filterModel->toggleTestResultType(static_cast<ResultType>(action->data().value<int>()));
    navigateStateChanged();
}

} // namespace Internal
} // namespace Autotest
