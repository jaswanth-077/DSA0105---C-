#include "TaskEditDialog.h"
#include "../../database/DatabaseManager.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

TaskEditDialog::TaskEditDialog(QWidget* parent, const Task* existingTask)
    : QDialog(parent)
    , m_taskId(-1)
    , m_createdAt(QDateTime::currentDateTime())
{
    setWindowTitle(existingTask ? "Edit Academic Task" : "Create Academic Task");
    resize(520, 560);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    auto* titleHeader = new QLabel(existingTask ? "Edit Task Details" : "Create New Task", this);
    titleHeader->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A;");
    mainLayout->addWidget(titleHeader);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Title
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setPlaceholderText("e.g. Operating Systems Lab 3");
    formLayout->addRow("Task Title *:", m_titleEdit);

    // Description
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setPlaceholderText("Brief description of objectives and deliverables...");
    m_descriptionEdit->setMaximumHeight(80);
    formLayout->addRow("Description:", m_descriptionEdit);

    // Subject
    m_subjectCombo = new QComboBox(this);
    m_subjectCombo->setEditable(true);
    populateSubjects();
    formLayout->addRow("Subject *:", m_subjectCombo);

    // Type
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItems({"Assignment", "Exam Preparation", "Project", "Revision", "Personal Study"});
    formLayout->addRow("Task Type:", m_typeCombo);

    // Priority
    m_priorityCombo = new QComboBox(this);
    m_priorityCombo->addItems({"Low", "Medium", "High", "Critical"});
    m_priorityCombo->setCurrentText("Medium");
    formLayout->addRow("Priority:", m_priorityCombo);

    // Deadline
    m_deadlineEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(3), this);
    m_deadlineEdit->setCalendarPopup(true);
    m_deadlineEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    formLayout->addRow("Deadline *:", m_deadlineEdit);

    // Estimated Duration
    m_estimatedDurationSpin = new QDoubleSpinBox(this);
    m_estimatedDurationSpin->setRange(0.25, 200.0);
    m_estimatedDurationSpin->setSingleStep(0.5);
    m_estimatedDurationSpin->setValue(3.0);
    m_estimatedDurationSpin->setSuffix(" hours");
    formLayout->addRow("Estimated Work:", m_estimatedDurationSpin);

    // Remaining Duration
    m_remainingDurationSpin = new QDoubleSpinBox(this);
    m_remainingDurationSpin->setRange(0.0, 200.0);
    m_remainingDurationSpin->setSingleStep(0.5);
    m_remainingDurationSpin->setValue(3.0);
    m_remainingDurationSpin->setSuffix(" hours");
    formLayout->addRow("Remaining Work:", m_remainingDurationSpin);

    // Completion Percentage
    m_completionSpin = new QSpinBox(this);
    m_completionSpin->setRange(0, 100);
    m_completionSpin->setSuffix(" %");
    m_completionSpin->setValue(0);
    connect(m_completionSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &TaskEditDialog::onPercentageChanged);
    formLayout->addRow("Completion:", m_completionSpin);

    // Status
    m_statusCombo = new QComboBox(this);
    m_statusCombo->addItems({"Not Started", "In Progress", "Completed", "Delayed"});
    formLayout->addRow("Status:", m_statusCombo);

    mainLayout->addLayout(formLayout);

    // Error Label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #DC2626; font-weight: 500; font-size: 12px;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setProperty("class", "secondaryButton");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* saveBtn = new QPushButton(existingTask ? "Save Changes" : "Create Task", this);
    saveBtn->setProperty("class", "primaryButton");
    connect(saveBtn, &QPushButton::clicked, this, &TaskEditDialog::onSaveClicked);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    // Populate existing values if editing
    if (existingTask) {
        m_taskId = existingTask->id();
        m_createdAt = existingTask->createdAt();
        m_titleEdit->setText(existingTask->title());
        m_descriptionEdit->setPlainText(existingTask->description());
        m_subjectCombo->setEditText(existingTask->subject());
        m_typeCombo->setCurrentText(Task::typeToString(existingTask->type()));
        m_priorityCombo->setCurrentText(Task::priorityToString(existingTask->priority()));
        m_deadlineEdit->setDateTime(existingTask->deadline());
        m_estimatedDurationSpin->setValue(existingTask->estimatedDuration());
        m_remainingDurationSpin->setValue(existingTask->remainingDuration());
        m_completionSpin->setValue(existingTask->completionPercentage());
        m_statusCombo->setCurrentText(Task::statusToString(existingTask->status()));
    }
}

void TaskEditDialog::populateSubjects()
{
    m_subjectCombo->clear();
    QVector<QString> subjects = DatabaseManager::instance().getDistinctSubjects();
    if (subjects.isEmpty()) {
        subjects = {"Computer Science", "Mathematics", "Physics", "Software Engineering", "Humanities"};
    }
    for (const auto& s : subjects) {
        m_subjectCombo->addItem(s);
    }
}

void TaskEditDialog::onPercentageChanged(int val)
{
    if (val == 100) {
        m_statusCombo->setCurrentText("Completed");
        m_remainingDurationSpin->setValue(0.0);
    } else if (val > 0 && m_statusCombo->currentText() == "Not Started") {
        m_statusCombo->setCurrentText("In Progress");
        double ratio = static_cast<double>(val) / 100.0;
        m_remainingDurationSpin->setValue(m_estimatedDurationSpin->value() * (1.0 - ratio));
    }
}

void TaskEditDialog::onSaveClicked()
{
    QString title = m_titleEdit->text().trimmed();
    QString subject = m_subjectCombo->currentText().trimmed();

    if (title.isEmpty()) {
        m_errorLabel->setText("Please enter a valid task title.");
        m_errorLabel->show();
        m_titleEdit->setFocus();
        return;
    }
    if (subject.isEmpty()) {
        m_errorLabel->setText("Please specify a subject or academic module.");
        m_errorLabel->show();
        m_subjectCombo->setFocus();
        return;
    }
    if (m_estimatedDurationSpin->value() <= 0.0) {
        m_errorLabel->setText("Estimated duration must be greater than zero.");
        m_errorLabel->show();
        return;
    }

    accept();
}

Task TaskEditDialog::getTask() const
{
    return Task(
        m_taskId,
        m_titleEdit->text().trimmed(),
        m_descriptionEdit->toPlainText().trimmed(),
        m_subjectCombo->currentText().trimmed(),
        Task::stringToType(m_typeCombo->currentText()),
        Task::stringToPriority(m_priorityCombo->currentText()),
        m_createdAt,
        m_deadlineEdit->dateTime(),
        m_estimatedDurationSpin->value(),
        m_remainingDurationSpin->value(),
        m_completionSpin->value(),
        Task::stringToStatus(m_statusCombo->currentText())
    );
}
