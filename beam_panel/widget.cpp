#include "widget.h"
#include "maintab.h"
#include "beam_panel.h"
#include <QtWidgets>

Widget::Widget(QWidget* parent) : QWidget(parent)
{
	buildUi();
	buildLayout();
	connectSignals();

	setWindowTitle(tr("Test"));
	resize(1000, 600);
}

void Widget::buildUi()
{
	m_tabs = new QTabWidget(this);
	m_mainTab = new MainTab(m_tabs);
	m_beam_panel_tab = new BeamPanel(m_tabs);

	m_tabs->addTab(m_mainTab, tr("Main"));
	m_tabs->addTab(m_beam_panel_tab, tr("Beam Panel"));
}

void Widget::buildLayout()
{
	auto* rootLayout = new QVBoxLayout(this);
	rootLayout->setContentsMargins(0, 0, 0, 0);
	rootLayout->addWidget(m_tabs, 1);
}

void Widget::connectSignals()
{
	// пока пусто; здесь будут connect'ы
}