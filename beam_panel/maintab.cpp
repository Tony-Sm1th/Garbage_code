#include "maintab.h"
#include <QtWidgets>

MainTab::MainTab(QWidget *parent)
    : QWidget(parent)
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 4);

    m_textEdit     = new QTextEdit(this);
    m_randomButton = new QPushButton(tr("Random"), this);

    rootLayout->addWidget(m_textEdit, 1);
    rootLayout->addWidget(m_randomButton, 0);
}