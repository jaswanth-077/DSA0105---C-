#include "TaskManagerView.h"
#include "../../services/TaskService.h"
#include "../../services/DeadlineAnalyzer.h"
#include "../../database/DatabaseManager.h"
#include "dialogs/TaskEditDialog.h"
#include "dialogs/LogSessionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QProgressBar>

TaskManagerView::TaskManagerView(QWidget* parent)
    : QWidget(parent)
    , m_selectedTaskId(-1)
{
    setupUi();
    refreshTasks();
}

void TaskManagerView::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 30);
    mainLayout->setSpacing(20);

    // Header Row
    auto* headerLayout = new QHBoxLayout();
    auto* titleContainer = new QWidget(this);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);

    auto* title = new QLabel("Academic Task Management", titleContainer);
    title->setProperty("class", "pageTitle");
    auto* subtitle = new QLabel("Track, filter, and prioritize assignments, examination preps, and projects.", titleContainer);
    subtitle->setProperty("class", "pageSubtitle");

    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    headerLayout->addWidget(titleContainer);
    headerLayout->addStretch();

    auto* newBtn = new QPushButton("+ New Task", this);
    newBtn->setProperty("class", "primaryButton");
    connect(newBtn, &QPushButton::clicked, this, &TaskManagerView::onNewTaskClicked);
    headerLayout->addWidget(newBtn);
    mainLayout->addLayout(headerLayout);

    // Filter Bar Card
    auto* filterCard = new QFrame(this);
    filterCard->setProperty("class", "cardFrame");
    auto* filterLayout = new QHBoxLayout(filterCard);
    filterLayout->setContentsMargins(16, 12, 16, 12);
    filterLayout->setSpacing(12);

    m_searchEdit = new QLineEdit(filterCard);
    m_searchEdit->setPlaceholderText("🔍 Search tasks, descriptions, or subjects...");
    m_searchEdit->setMinimumWidth(220);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &TaskManagerView::onFilterChanged);

    m_subjectFilter = new QComboBox(filterCard);
    m_subjectFilter->addItem("All Subjects");
    connect(m_subjectFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskManagerView::onFilterChanged);

    m_priorityFilter = new QComboBox(filterCard);
    m_priorityFilter->addItems({"All Priorities", "Critical", "High", "Medium", "Low"});
    connect(m_priorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskManagerView::onFilterChanged);

    m_statusFilter = new QComboBox(filterCard);
    m_statusFilter->addItems({"All Statuses", "Not Started", "In Progress", "Completed", "Delayed"});
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskManagerView::onFilterChanged);

    m_sortCombo = new QComboBox(filterCard);
    m_sortCombo->addItems({
        "Deadline (Earliest First)",
        "Deadline (Latest First)",
        "Priority (High to Low)",
        "Priority (Low to High)",
        "Remaining Workload",
        "Completion %"
    });
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskManagerView::onFilterChanged);

    filterLayout->addWidget(m_searchEdit, 2);
    filterLayout->addWidget(m_subjectFilter, 1);
    filterLayout->addWidget(m_priorityFilter, 1);
    filterLayout->addWidget(m_statusFilter, 1);
    filterLayout->addWidget(m_sortCombo, 1);
    mainLayout->addWidget(filterCard);

    // Center Content: Table on Left + Details Panel on Right
    auto* centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(20);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"Title", "Subject", "Type", "Priority", "Deadline", "Progress", "Status"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &TaskManagerView::onTaskSelectionChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &TaskManagerView::onEditTaskClicked);
    centerLayout->addWidget(m_table, 3);

    // Detail Drawer / Panel
    m_detailPanel = new QFrame(this);
    m_detailPanel->setProperty("class", "cardFrame");
    m_detailPanel->setFixedWidth(340);
    auto* dtLayout = new QVBoxLayout(m_detailPanel);
    dtLayout->setContentsMargins(18, 18, 18, 18);
    dtLayout->setSpacing(12);

    auto* dtHeader = new QLabel("Task Details", m_detailPanel);
    dtHeader->setProperty("class", "sectionTitle");
    dtLayout->addWidget(dtHeader);

    m_detailTitle = new QLabel("Select a task to view details", m_detailPanel);
    m_detailTitle->setWordWrap(true);
    m_detailTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #0F172A;");
    dtLayout->addWidget(m_detailTitle);

    m_detailSubject = new QLabel("", m_detailPanel);
    m_detailSubject->setStyleSheet("color: #4F46E5; font-weight: 600; font-size: 12px;");
    dtLayout->addWidget(m_detailSubject);

    auto* metaGrid = new QGridLayout();
    metaGrid->setSpacing(6);

    auto makeMetaRow = [this, metaGrid](int row, const QString& labelText, QLabel*& valLabel) {
        auto* l = new QLabel(labelText, m_detailPanel);
        l->setStyleSheet("color: #64748B; font-size: 11px;");
        valLabel = new QLabel("—", m_detailPanel);
        valLabel->setStyleSheet("font-weight: 600; font-size: 12px; color: #1E293B;");
        metaGrid->addWidget(l, row, 0);
        metaGrid->addWidget(valLabel, row, 1);
    };

    makeMetaRow(0, "Type:", m_detailType);
    makeMetaRow(1, "Priority:", m_detailPriority);
    makeMetaRow(2, "Deadline:", m_detailDeadline);
    makeMetaRow(3, "Workload:", m_detailHours);
    makeMetaRow(4, "Progress:", m_detailProgress);
    makeMetaRow(5, "Status:", m_detailStatus);
    dtLayout->addLayout(metaGrid);

    auto* descHeader = new QLabel("Description:", m_detailPanel);
    descHeader->setStyleSheet("font-size: 11px; color: #64748B; font-weight: 600; text-transform: uppercase;");
    dtLayout->addWidget(descHeader);

    m_detailDescription = new QTextEdit(m_detailPanel);
    m_detailDescription->setReadOnly(true);
    m_detailDescription->setStyleSheet("background-color: #F8FAFC; border: 1px solid #E2E8F0; border-radius: 6px; font-size: 12px;");
    m_detailDescription->setMaximumHeight(90);
    dtLayout->addWidget(m_detailDescription);

    auto* riskHeader = new QLabel("Deadline Risk Analysis:", m_detailPanel);
    riskHeader->setStyleSheet("font-size: 11px; color: #64748B; font-weight: 600; text-transform: uppercase;");
    dtLayout->addWidget(riskHeader);

    m_detailRiskExplanation = new QLabel("—", m_detailPanel);
    m_detailRiskExplanation->setWordWrap(true);
    m_detailRiskExplanation->setStyleSheet("font-size: 12px; color: #475569; background-color: #F8FAFC; padding: 8px; border-radius: 6px;");
    dtLayout->addWidget(m_detailRiskExplanation);

    dtLayout->addStretch();

    // Action buttons in detail panel
    auto* btnGrid = new QGridLayout();
    btnGrid->setSpacing(8);

    m_logSessionBtn = new QPushButton("⏱️ Log Study", m_detailPanel);
    m_logSessionBtn->setProperty("class", "secondaryButton");
    connect(m_logSessionBtn, &QPushButton::clicked, this, &TaskManagerView::onLogSessionClicked);

    m_editBtn = new QPushButton("✏️ Edit", m_detailPanel);
    m_editBtn->setProperty("class", "secondaryButton");
    connect(m_editBtn, &QPushButton::clicked, this, &TaskManagerView::onEditTaskClicked);

    m_completeBtn = new QPushButton("✅ Complete", m_detailPanel);
    m_completeBtn->setProperty("class", "successButton");
    connect(m_completeBtn, &QPushButton::clicked, this, &TaskManagerView::onCompleteTaskClicked);

    m_deleteBtn = new QPushButton("🗑️ Delete", m_detailPanel);
    m_deleteBtn->setProperty("class", "dangerButton");
    connect(m_deleteBtn, &QPushButton::clicked, this, &TaskManagerView::onDeleteTaskClicked);

    btnGrid->addWidget(m_logSessionBtn, 0, 0);
    btnGrid->addWidget(m_editBtn, 0, 1);
    btnGrid->addWidget(m_completeBtn, 1, 0);
    btnGrid->addWidget(m_deleteBtn, 1, 1);
    dtLayout->addLayout(btnGrid);

    centerLayout->addWidget(m_detailPanel, 1);
    mainLayout->addLayout(centerLayout);
}

void TaskManagerView::refreshTasks()
{
    // Update subjects filter
    QString currentSub = m_subjectFilter->currentText();
    m_subjectFilter->blockSignals(true);
    m_subjectFilter->clear();
    m_subjectFilter->addItem("All Subjects");
    QVector<QString> subjects = DatabaseManager::instance().getDistinctSubjects();
    for (const auto& s : subjects) {
        m_subjectFilter->addItem(s);
    }
    int idx = m_subjectFilter->findText(currentSub);
    m_subjectFilter->setCurrentIndex(idx >= 0 ? idx : 0);
    m_subjectFilter->blockSignals(false);

    onFilterChanged();
}

void TaskManagerView::onFilterChanged()
{
    QString search = m_searchEdit->text().trimmed();
    QString subject = m_subjectFilter->currentText();
    QString priority = m_priorityFilter->currentText();
    QString status = m_statusFilter->currentText();
    QString sortBy = m_sortCombo->currentText();

    QVector<Task> tasks = TaskService::instance().filterTasks(search, subject, priority, status, sortBy);
    m_table->setRowCount(tasks.size());

    int selectedRow = -1;
    for (int r = 0; r < tasks.size(); ++r) {
        const auto& t = tasks[r];

        auto* titleItem = new QTableWidgetItem(t.title());
        titleItem->setData(Qt::UserRole, t.id());
        m_table->setItem(r, 0, titleItem);

        m_table->setItem(r, 1, new QTableWidgetItem(t.subject()));
        m_table->setItem(r, 2, new QTableWidgetItem(Task::typeToString(t.type())));

        auto* prioItem = new QTableWidgetItem(Task::priorityToString(t.priority()));
        prioItem->setForeground(Task::priorityColor(t.priority()));
        prioItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        m_table->setItem(r, 3, prioItem);

        QString dlStr = t.deadline().toString("yyyy-MM-dd HH:mm");
        if (t.isOverdue()) {
            dlStr += " (Overdue)";
        }
        auto* dlItem = new QTableWidgetItem(dlStr);
        if (t.isOverdue()) {
            dlItem->setForeground(QColor("#DC2626"));
            dlItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        }
        m_table->setItem(r, 4, dlItem);

        // Progress cell with percentage
        auto* progItem = new QTableWidgetItem(QString("%1%").arg(t.completionPercentage()));
        progItem->setTextAlignment(Qt::AlignCenter);
        if (t.isCompleted()) {
            progItem->setForeground(QColor("#10B981"));
            progItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        }
        m_table->setItem(r, 5, progItem);

        auto* statItem = new QTableWidgetItem(Task::statusToString(t.status()));
        statItem->setForeground(Task::statusColor(t.status()));
        statItem->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        m_table->setItem(r, 6, statItem);

        if (t.id() == m_selectedTaskId) {
            selectedRow = r;
        }
    }

    if (selectedRow >= 0) {
        m_table->selectRow(selectedRow);
    } else if (m_table->rowCount() > 0) {
        m_table->selectRow(0);
    } else {
        updateDetailPanel(nullptr);
    }
}

void TaskManagerView::onTaskSelectionChanged()
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_table->rowCount()) {
        updateDetailPanel(nullptr);
        return;
    }

    QTableWidgetItem* item = m_table->item(row, 0);
    if (!item) return;

    int taskId = item->data(Qt::UserRole).toInt();
    m_selectedTaskId = taskId;

    auto opt = TaskService::instance().getTask(taskId);
    if (opt) {
        updateDetailPanel(&*opt);
    } else {
        updateDetailPanel(nullptr);
    }
}

void TaskManagerView::updateDetailPanel(const Task* task)
{
    if (!task) {
        m_selectedTaskId = -1;
        m_detailTitle->setText("Select a task to view details");
        m_detailSubject->setText("");
        m_detailType->setText("—");
        m_detailPriority->setText("—");
        m_detailDeadline->setText("—");
        m_detailHours->setText("—");
        m_detailProgress->setText("—");
        m_detailStatus->setText("—");
        m_detailDescription->setPlainText("");
        m_detailRiskExplanation->setText("—");
        m_editBtn->setEnabled(false);
        m_logSessionBtn->setEnabled(false);
        m_completeBtn->setEnabled(false);
        m_deleteBtn->setEnabled(false);
        return;
    }

    m_selectedTaskId = task->id();
    m_detailTitle->setText(task->title());
    m_detailSubject->setText(task->subject());
    m_detailType->setText(Task::typeToString(task->type()));

    m_detailPriority->setText(Task::priorityToString(task->priority()));
    m_detailPriority->setStyleSheet(QString("font-weight: bold; color: %1;").arg(Task::priorityColor(task->priority()).name()));

    m_detailDeadline->setText(task->deadline().toString("MMM d, yyyy HH:mm"));
    m_detailHours->setText(QString("%1h remaining of %2h total")
                               .arg(QString::number(task->remainingDuration(), 'f', 1))
                               .arg(QString::number(task->estimatedDuration(), 'f', 1)));

    m_detailProgress->setText(QString("%1% completed").arg(task->completionPercentage()));
    m_detailStatus->setText(Task::statusToString(task->status()));
    m_detailStatus->setStyleSheet(QString("font-weight: bold; color: %1;").arg(Task::statusColor(task->status()).name()));

    m_detailDescription->setPlainText(task->description().isEmpty() ? "No description provided." : task->description());

    DeadlineRisk risk = DeadlineAnalyzer::instance().analyzeTask(*task);
    QString riskText = QString("Classification: %1\n%2\n\nRecommendation: %3")
                           .arg(DeadlineRisk::levelToString(risk.level()))
                           .arg(risk.explanation())
                           .arg(risk.recommendation());
    m_detailRiskExplanation->setText(riskText);
    m_detailRiskExplanation->setStyleSheet(QString("background-color: %1; color: #1E293B; padding: 10px; border-radius: 6px; font-size: 11px;")
                                               .arg(DeadlineRisk::levelBackgroundColor(risk.level()).name()));

    m_editBtn->setEnabled(true);
    m_logSessionBtn->setEnabled(true);
    m_completeBtn->setEnabled(!task->isCompleted());
    m_deleteBtn->setEnabled(true);
}

void TaskManagerView::onNewTaskClicked()
{
    TaskEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Task t = dlg.getTask();
        QString err;
        int newId = TaskService::instance().createTask(t, err);
        if (newId > 0) {
            refreshTasks();
            selectTaskById(newId);
            emit dataChanged();
        } else {
            QMessageBox::warning(this, "Task Creation Failed", err);
        }
    }
}

void TaskManagerView::onEditTaskClicked()
{
    if (m_selectedTaskId == -1) return;
    auto opt = TaskService::instance().getTask(m_selectedTaskId);
    if (!opt) return;

    TaskEditDialog dlg(this, &*opt);
    if (dlg.exec() == QDialog::Accepted) {
        Task t = dlg.getTask();
        QString err;
        if (TaskService::instance().updateTask(t, err)) {
            refreshTasks();
            selectTaskById(t.id());
            emit dataChanged();
        } else {
            QMessageBox::warning(this, "Update Failed", err);
        }
    }
}

void TaskManagerView::onDeleteTaskClicked()
{
    if (m_selectedTaskId == -1) return;
    auto opt = TaskService::instance().getTask(m_selectedTaskId);
    if (!opt) return;

    auto reply = QMessageBox::question(
        this,
        "Confirm Task Deletion",
        QString("Are you sure you want to permanently delete \"%1\"?\nThis will remove associated schedule entries and study sessions.").arg(opt->title()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        TaskService::instance().deleteTask(m_selectedTaskId);
        m_selectedTaskId = -1;
        refreshTasks();
        emit dataChanged();
    }
}

void TaskManagerView::onCompleteTaskClicked()
{
    if (m_selectedTaskId == -1) return;
    TaskService::instance().markTaskCompleted(m_selectedTaskId);
    refreshTasks();
    selectTaskById(m_selectedTaskId);
    emit dataChanged();
}

void TaskManagerView::onLogSessionClicked()
{
    if (m_selectedTaskId == -1) return;
    LogSessionDialog dlg(this, m_selectedTaskId);
    if (dlg.exec() == QDialog::Accepted) {
        StudySession s = dlg.getStudySession();
        DatabaseManager::instance().addStudySession(s);

        // Deduct logged hours from task's remaining duration
        auto opt = TaskService::instance().getTask(m_selectedTaskId);
        if (opt) {
            Task t = *opt;
            double loggedHours = static_cast<double>(dlg.getLoggedMinutes()) / 60.0;
            double newRemaining = std::max(0.0, t.remainingDuration() - loggedHours);
            t.setRemainingDuration(newRemaining);

            if (t.estimatedDuration() > 0.0) {
                int newPct = std::min(100, static_cast<int>(((t.estimatedDuration() - newRemaining) / t.estimatedDuration()) * 100.0));
                t.setCompletionPercentage(newPct);
            }
            QString err;
            TaskService::instance().updateTask(t, err);
        }

        refreshTasks();
        selectTaskById(m_selectedTaskId);
        emit dataChanged();
        QMessageBox::information(this, "Session Logged", QString("Successfully logged %1 minutes of focused study!").arg(dlg.getLoggedMinutes()));
    }
}

void TaskManagerView::selectTaskById(int taskId)
{
    m_selectedTaskId = taskId;
    for (int r = 0; r < m_table->rowCount(); ++r) {
        if (m_table->item(r, 0)->data(Qt::UserRole).toInt() == taskId) {
            m_table->selectRow(r);
            break;
        }
    }
}
