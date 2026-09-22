#include "widget.h"
#include "maintab.h"
#include "beam_panel.h"
#include "data_emulator.h"

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
	// create the emulator (owned by this widget)
	m_emulator = new DataEmulator(this);
}

void Widget::buildLayout()
{
	auto* rootLayout = new QVBoxLayout(this);
	rootLayout->setContentsMargins(0, 0, 0, 0);
	rootLayout->addWidget(m_tabs, 1);
}

void Widget::connectSignals()
{
	connect(m_emulator, &DataEmulator::histograms_ready, m_beam_panel_tab,
			&BeamPanel::on_histograms_ready);

	// start: once per second
	m_emulator->start(1000);
}