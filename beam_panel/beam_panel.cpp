#include "beam_panel.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QDebug>
#include <iostream>

beam_panel::beam_panel(QWidget* parent) : QWidget(parent)
{
	build_ui();
	build_layout();
	connect_signals();
}

void beam_panel::build_ui()
{
	m_histogram_plot = new QTextEdit(this);
	m_random_button = new QPushButton(tr("Random"), this);

	//group one: four view buttons
	m_view_group = new QButtonGroup(this);
	m_view_group->setExclusive(true); // only one active

	for(int i = 1; i <= 4; ++i)
	{
		QToolButton* btn = new QToolButton(this);
		btn->setText(QString::number(i));
		btn->setCheckable(true);
		btn->setAutoRaise(true);
		btn->setFixedSize(24, 24);

		m_view_group->addButton(btn, i); // add with id = i
		m_view_buttons.append(btn);
	}
	m_view_buttons[0]->setChecked(true); // default button
}

void beam_panel::build_layout()
{
	//separate window into two sections: histogram and buttons
	auto* root_layout = new QHBoxLayout(this);
	root_layout->setContentsMargins(4, 4, 4, 4);
	root_layout->addWidget(m_histogram_plot, 1); //add histogram

	//make button section formatted in vertical style
	auto* side_layout = new QVBoxLayout();
	side_layout->setSpacing(6);

	//group one: four view buttons
	auto* view_group_layout = new QHBoxLayout();
	view_group_layout->setSpacing(2);
	for(QToolButton* btn: m_view_buttons)
	{
		view_group_layout->addWidget(btn);
	}

	view_group_layout->addStretch();

	side_layout->addLayout(view_group_layout);

	//group two
	side_layout->addWidget(m_random_button);

	root_layout->addLayout(side_layout, 0);
}

void beam_panel::connect_signals()
{
	connect(m_view_group, &QButtonGroup::idClicked, this, &beam_panel::on_view_changed);
}

void beam_panel::on_view_changed(int a_id)
{
	// std::cout << "std::cout: view changed to " << a_id << std::endl;
	// qInfo() << "qInfo: view changed to" << a_id;
	// qWarning() << "qWarning: view changed to" << a_id;
	// qCritical() << "qCritical: view changed to" << a_id;
	// qDebug() << "qDebug: view changed to" << a_id;

	qDebug() << "view changed to" << a_id;
}