#include "StatCard.h"

StatCard::StatCard(const QString& icon,
                   const QString& label,
                   const QString& value,
                   const QString& subtext,
                   const QString& badgeColor,
                   QWidget* parent)
    : QFrame(parent)
{
    setProperty("class", "cardFrame cardFrameHover");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(6);

    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    m_iconLabel = new QLabel(icon, this);
    m_iconLabel->setStyleSheet(QString(
        "background-color: %115; color: %1; border-radius: 6px; padding: 4px 8px; font-size: 14px; font-weight: bold;"
    ).arg(badgeColor));

    m_labelLabel = new QLabel(label, this);
    m_labelLabel->setProperty("class", "cardLabel");

    topLayout->addWidget(m_labelLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_iconLabel);

    m_valueLabel = new QLabel(value, this);
    m_valueLabel->setProperty("class", "cardValue");

    m_subtextLabel = new QLabel(subtext, this);
    m_subtextLabel->setProperty("class", "cardSubtext");
    m_subtextLabel->setWordWrap(true);

    layout->addLayout(topLayout);
    layout->addWidget(m_valueLabel);
    layout->addWidget(m_subtextLabel);
}

void StatCard::setValue(const QString& value)
{
    m_valueLabel->setText(value);
}

void StatCard::setSubtext(const QString& subtext)
{
    m_subtextLabel->setText(subtext);
}
