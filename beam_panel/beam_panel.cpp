#include "beam_panel.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QStackedWidget>
#include <QSplitter>

BeamPanel::BeamPanel(QWidget* parent) : QWidget(parent)
{
	build_ui();
	build_layout();
	connect_signals();
}

void BeamPanel::build_ui()
{
	//left side
	m_stack = new QStackedWidget(this);

	// page 0: view A
	m_view_a = new QTextEdit(m_stack);
	m_view_a->setPlainText("View A");
	m_stack->addWidget(m_view_a);

	// page 1: view B
	m_view_b = new QTextEdit(m_stack);
	m_view_b->setPlainText("View B");
	m_stack->addWidget(m_view_b);

	// page 2: split A (top) + B (bottom)
	auto* split = new QSplitter(Qt::Vertical, m_stack);
	auto* split_a = new QTextEdit(split);
	split_a->setPlainText("Split — A");
	auto* split_b = new QTextEdit(split);
	split_b->setPlainText("Split — B");
	split->addWidget(split_a);
	split->addWidget(split_b);
	m_view_split = split;
	m_stack->addWidget(m_view_split);

	// page 3: sum
	m_view_sum = new QTextEdit(m_stack);
	m_view_sum->setPlainText("Sum A + B");
	m_stack->addWidget(m_view_sum);

	m_stack->setCurrentIndex(0); // по умолчанию — view A

	//group 1: four view buttons
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

	//group 2: Center of gravity
	m_center_of_gravity_group = new QGroupBox(tr("Center of gravity"), this);
	m_center_of_gravity_label = new QLabel(tr("--"), m_center_of_gravity_group);
	auto* cog_layout = new QVBoxLayout(m_center_of_gravity_group);
	cog_layout->addWidget(m_center_of_gravity_label);

	//group 3: Select capacitor
	m_capacitor_group = new QGroupBox(tr("Charge in capacitor"), this);
	m_capacitor_combo_box = new QComboBox(m_capacitor_group);
	m_capacitor_combo_box->addItem(tr("12.5 pC"), int(Capacitor::C12p5));
	m_capacitor_combo_box->addItem(tr("50 pC"), int(Capacitor::C50));
	m_capacitor_combo_box->addItem(tr("100 pC"), int(Capacitor::C100));
	m_capacitor_combo_box->addItem(tr("150 pC"), int(Capacitor::C150));
	auto* capacitor_layout = new QVBoxLayout(m_capacitor_group);
	capacitor_layout->addWidget(m_capacitor_combo_box);
	m_capacitor_combo_box->setCurrentIndex(1);

	//group 4: Conversion
	m_conversion_group = new QGroupBox(tr("Conversion"), this);
	m_conversion_label = new QLabel(tr("Conversion interval"), m_conversion_group);
	m_conversion_spin_box = new QSpinBox(m_conversion_group);
	m_conversion_spin_box->setRange(DEFAULT_CONV_MIN_RANGE, DEFAULT_CONV_MAX_RANGE);
	m_conversion_spin_box->setValue(DEFAULT_CONV_VALUE);
	m_conversion_spin_box->setSuffix(tr(" us"));
	auto* conv_layout = new QVBoxLayout(m_conversion_group);
	conv_layout->addWidget(m_conversion_label);
	conv_layout->addWidget(m_conversion_spin_box);

	//group 5: Trigger
	m_trigger_group = new QGroupBox(tr("Trigger"), this);
	m_mode_trigger_label = new QLabel(tr("Trigger mode"), m_trigger_group);
	m_mode_trigger_combo_box = new QComboBox(m_trigger_group);
	m_mode_trigger_combo_box->addItem(tr("Hardware"), int(TriggerMode::Hardware));
	m_mode_trigger_combo_box->addItem(tr("Software"), int(TriggerMode::Software));
	m_mode_trigger_check_box = new QCheckBox(tr("Pulse mode"), m_trigger_group);
	m_mode_trigger_level_label = new QLabel(tr("Level"), m_trigger_group);
	m_mode_trigger_level_spin_box = new QSpinBox(m_trigger_group);
	m_mode_trigger_level_spin_box->setRange(DEFAULT_LEVEL_MIN_RANGE, DEFAULT_LEVEL_MAX_RANGE);
	m_mode_trigger_level_spin_box->setValue(DEFAULT_LEVEL_VALUE);
	m_mode_trigger_level_spin_box->setSuffix(tr(" LSB"));
	auto* trigger_layout = new QVBoxLayout(m_trigger_group);
	trigger_layout->addWidget(m_mode_trigger_label);
	trigger_layout->addWidget(m_mode_trigger_combo_box);
	trigger_layout->addWidget(m_mode_trigger_check_box);
	trigger_layout->addWidget(m_mode_trigger_level_label);
	trigger_layout->addWidget(m_mode_trigger_level_spin_box);

	//group 6: Noise background
	m_noise_background_group = new QGroupBox(tr("Noise cancellation"), this);
	m_noise_background_btn = new QPushButton(tr("Accumulate"), m_noise_background_group);
	m_noise_background_check_box =
		new QCheckBox(tr("Noise cancellation"), m_noise_background_group);
	auto* noise_background_layout = new QVBoxLayout(m_noise_background_group);
	noise_background_layout->addWidget(m_noise_background_btn);
	noise_background_layout->addWidget(m_noise_background_check_box);

	//group 7: Set parameters
	m_parameters_send_group = new QGroupBox(tr("Set parameters"), this);
	m_parameters_send_btn = new QPushButton(tr("Set"), m_parameters_send_group);
	// Sets the background color to blue and text color to white
	//m_parameters_send_btn->setStyleSheet("QPushButton { background-color: blue; color: white; }");
	auto* parameters_layout = new QVBoxLayout(m_parameters_send_group);
	parameters_layout->addWidget(m_parameters_send_btn);

	//group 8: Reading
	m_reading_group = new QGroupBox(tr("Reading"), this);
	m_reading_single_read_btn = new QPushButton(tr("Single read"), m_reading_group);
	m_reading_continuous_read_label = new QLabel(tr("Continuous read"), m_reading_group);
	m_reading_continuous_read_btn = new QPushButton(tr("Start"), m_reading_group);
	m_reading_continuous_read_btn->setCheckable(true);
	m_reading_spin_box = new QSpinBox(m_reading_group);
	m_reading_spin_box->setRange(DEFAULT_COMMANDS_MIN_RANGE, DEFAULT_COMMANDS_MAX_RANGE);
	m_reading_spin_box->setValue(DEFAULT_COMMANDS_VALUE);
	m_reading_spin_box->setSuffix(tr(" ms"));
	auto* reading_layout = new QVBoxLayout(m_reading_group);
	reading_layout->addWidget(m_reading_single_read_btn);
	reading_layout->addWidget(m_reading_continuous_read_label);
	reading_layout->addWidget(m_reading_spin_box);
	reading_layout->addWidget(m_reading_continuous_read_btn);
}

void BeamPanel::build_layout()
{
	//separate window into two sections: histogram and buttons
	auto* root_layout = new QHBoxLayout(this);
	root_layout->setContentsMargins(4, 4, 4, 4);
	//root_layout->addWidget(m_histogram_plot, 1); //add histogram
	root_layout->addWidget(m_stack, 1); //add histogram

	//make button section formatted in vertical style
	auto* side_layout = new QVBoxLayout();
	//side_layout->setSpacing(6);

	//group 1: four view buttons
	auto* view_group_layout = new QHBoxLayout();
	//view_group_layout->setSpacing(2);
	for(QToolButton* btn: m_view_buttons)
	{
		view_group_layout->addWidget(btn);
	}

	side_layout->addLayout(view_group_layout);
	side_layout->addSpacing(12); // fixed spacing

	//group 2
	side_layout->addWidget(m_center_of_gravity_group);

	//group 3
	side_layout->addWidget(m_capacitor_group);

	//group 4
	side_layout->addWidget(m_conversion_group);

	//group 5
	side_layout->addWidget(m_trigger_group);

	//group 6
	side_layout->addWidget(m_noise_background_group);

	//group 7
	side_layout->addWidget(m_parameters_send_group);
	side_layout->addStretch();

	//group 8
	side_layout->addWidget(m_reading_group);

	//add right side
	root_layout->addLayout(side_layout, 0);
}

void BeamPanel::connect_signals()
{
	connect(m_view_group, &QButtonGroup::idClicked, this, &BeamPanel::on_view_changed);
	connect(m_parameters_send_btn, &QPushButton::clicked, this,
			&BeamPanel::on_send_parameters_clicked);
	connect(m_reading_single_read_btn, &QPushButton::clicked, this,
			&BeamPanel::on_single_read_clicked);
	connect(m_reading_continuous_read_btn, &QPushButton::toggled, this,
			&BeamPanel::on_continuous_read_toggled);
}

void BeamPanel::on_view_changed(int a_id)
{
	// std::cout << "std::cout: view changed to " << a_id << std::endl;
	// qInfo() << "qInfo: view changed to" << a_id;
	// qWarning() << "qWarning: view changed to" << a_id;
	// qCritical() << "qCritical: view changed to" << a_id;
	// qDebug() << "qDebug: view changed to" << a_id;
	//qDebug() << QString::number(m_conversion_spin_box->value());
	m_stack->setCurrentIndex(a_id - 1);
	qDebug() << "view changed to" << a_id;
}

void BeamPanel::on_send_parameters_clicked()
{
	emit parameters_ready(parameters());
}

void BeamPanel::on_single_read_clicked()
{
	emit single_read_requested();
}

void BeamPanel::on_continuous_read_toggled(bool on)
{
	if(on)
	{
		m_reading_continuous_read_btn->setText("Stop");
		emit continuous_read_requested(m_reading_spin_box->value());
	}
	else
	{
		m_reading_continuous_read_btn->setText("Start");
		emit continuous_read_stopped();
	}
}

void BeamPanel::on_noise_background_clicked()
{
	emit noise_accumulation_requested();
}

void BeamPanel::set_center_of_gravity(double a_value)
{
	m_cog_value = a_value;
	m_center_of_gravity_label->setText(QString::number(m_cog_value, 'f', 2));
}

BeamPanel::BeamParameters BeamPanel::parameters() const
{
	BeamParameters p;
	p.capacitor = Capacitor(m_capacitor_combo_box->currentData().toInt());
	p.conversion_us = m_conversion_spin_box->value();
	p.trigger_mode = TriggerMode(m_mode_trigger_combo_box->currentData().toInt());
	p.trigger_pulse = m_mode_trigger_check_box->isChecked();
	p.trigger_level_lsb = m_mode_trigger_level_spin_box->value();
	p.noise_cancellation = m_noise_background_check_box->isChecked();
	p.read_interval_ms = m_reading_spin_box->value();
	return p;
}