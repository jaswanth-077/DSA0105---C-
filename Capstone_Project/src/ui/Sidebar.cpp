#include "Sidebar.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QFrame>

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("sidebarWidget");
    setFixedWidth(240);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 20, 16, 20);
    layout->setSpacing(12);

    // Header / Brand Logo
    auto* brandLayout = new QHBoxLayout();
    brandLayout->setSpacing(10);

    auto* logoBox = new QLabel("📚", this);
    logoBox->setStyleSheet("font-size: 24px;");

    auto* titleContainer = new QWidget(this);
    auto* titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(2);

    auto* titleLabel = new QLabel("StudyPlanner", titleContainer);
    titleLabel->setObjectName("appLogoTitle");

    auto* subLabel = new QLabel("ACADEMIC PRO v1.0", titleContainer);
    subLabel->setObjectName("appLogoSub");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subLabel);

    brandLayout->addWidget(logoBox);
    brandLayout->addWidget(titleContainer);
    brandLayout->addStretch();
    layout->addLayout(brandLayout);

    layout->addSpacing(10);

    // Quick Action: + New Task Button
    auto* newTaskBtn = new QPushButton("+ New Task", this);
    newTaskBtn->setProperty("class", "primaryButton");
    newTaskBtn->setStyleSheet("background-color: #4F46E5; color: #FFFFFF; font-weight: bold; padding: 10px; border-radius: 8px;");
    connect(newTaskBtn, &QPushButton::clicked, this, &Sidebar::newTaskRequested);
    layout->addWidget(newTaskBtn);

    layout->addSpacing(10);

    // Navigation Buttons Group
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    struct NavItem {
        QString icon;
        QString text;
        int id;
    };

    QVector<NavItem> items = {
        {"📊", "Dashboard", 0},
        {"📝", "Tasks", 1},
        {"📅", "Recovery & Schedule", 2},
        {"📈", "Workload & Analytics", 3},
        {"⏱️", "Focus Timer", 4},
        {"⚙️", "Settings", 5}
    };

    m_navLayout = new QVBoxLayout();
    m_navLayout->setSpacing(6);

    for (const auto& item : items) {
        auto* btn = new QPushButton(QString("%1  %2").arg(item.icon, item.text), this);
        btn->setProperty("class", "sidebarButton");
        btn->setCheckable(true);
        m_buttonGroup->addButton(btn, item.id);
        m_navLayout->addWidget(btn);
        m_navButtons.append(btn);
    }

    connect(m_buttonGroup, &QButtonGroup::idClicked, this, &Sidebar::tabChanged);
    layout->addLayout(m_navLayout);

    layout->addStretch();

    // User Profile Pill at bottom
    auto* profileFrame = new QFrame(this);
    profileFrame->setStyleSheet("background-color: #1E293B; border-radius: 8px; padding: 10px;");
    auto* profileLayout = new QHBoxLayout(profileFrame);
    profileLayout->setContentsMargins(8, 8, 8, 8);
    profileLayout->setSpacing(10);

    auto* avatar = new QLabel("🎓", profileFrame);
    avatar->setStyleSheet("font-size: 20px;");

    auto* userInfo = new QWidget(profileFrame);
    auto* userLayout = new QVBoxLayout(userInfo);
    userLayout->setContentsMargins(0, 0, 0, 0);
    userLayout->setSpacing(2);

    auto* userName = new QLabel("Active Student", userInfo);
    userName->setStyleSheet("color: #FFFFFF; font-weight: 600; font-size: 12px;");

    auto* userSub = new QLabel("Fall Term • Offline", userInfo);
    userSub->setStyleSheet("color: #94A3B8; font-size: 11px;");

    userLayout->addWidget(userName);
    userLayout->addWidget(userSub);

    profileLayout->addWidget(avatar);
    profileLayout->addWidget(userInfo);
    profileLayout->addStretch();

    layout->addWidget(profileFrame);

    // Default select Dashboard
    if (!m_navButtons.isEmpty()) {
        m_navButtons[0]->setChecked(true);
    }
}

void Sidebar::setCurrentIndex(int index)
{
    if (index >= 0 && index < m_navButtons.size()) {
        m_navButtons[index]->setChecked(true);
    }
}
