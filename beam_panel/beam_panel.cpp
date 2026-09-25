#include "beam_panel.h"
#include "histogram_plot.h"
#include "heatmap_plot.h"
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
#include <QPainter>
#include <QPixmap>

static QIcon make_view_icon(int a_mode)
{
	const int S = 24;
	const int margin = 3;
	const int gap = 2;
	const int radius = 2;

	QPixmap pm(S, S);
	pm.fill(Qt::transparent);

	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(45, 90, 140)); // #2D5A8C, dark blue

	const int area = S - 2 * margin;   // 18
	const int half = (area - gap) / 2; // 8

	switch(a_mode)
	{
		case 0: // A only
			p.drawRoundedRect(margin, margin, area, half, radius, radius);
			break;

		case 1: // B only
			p.drawRoundedRect(margin, margin + half + gap, area, half, radius, radius);
			break;

		case 2: // split
			p.drawRoundedRect(margin, margin, area, half, radius, radius);
			p.drawRoundedRect(margin, margin + half + gap, area, half, radius, radius);
			break;
		case 3: // sum: square with a circle in the center
			// outer square
			p.setBrush(QColor(45, 90, 140)); // main color
			p.drawRoundedRect(margin, margin, area, area, radius, radius);

			// inner circle (different color)
			p.setBrush(QColor(200, 220, 240)); // light blue, or white
			p.drawEllipse(
				QRectF(margin + area * 0.25, margin + area * 0.25, area * 0.5, area * 0.5));
			break;
	}

	return QIcon(pm);
}

BeamPanel::BeamPanel(QWidget* parent) : QWidget(parent)
{
	setMinimumWidth(MIN_PANEL_WIDTH);
	build_ui();
	build_layout();
	connect_signals();
	//make current in accordance with capacitor and time interval
	apply_current_scale_to_plots();
}

void BeamPanel::build_ui()
{
	//left side
	m_stack = new QStackedWidget(this);

	// page 0: splitter with A (top) and B (bottom)
	m_splitter = new QSplitter(Qt::Vertical, m_stack);

	m_hist_a = new HistogramPlot(m_splitter);
	m_hist_a->setTitle(tr("A"));
	m_hist_b = new HistogramPlot(m_splitter);
	m_hist_b->setTitle(tr("B"));

	m_splitter->addWidget(m_hist_a);
	m_splitter->addWidget(m_hist_b);
	m_splitter->setSizes({1, 1}); // both visible initially

	m_stack->addWidget(m_splitter); // page 0

	// page 1: intensity heatmap
	m_heatmap = new HeatmapPlot(m_stack);
	m_heatmap->setTitle(tr("Beam intensity"));
	m_stack->addWidget(m_heatmap);

	m_stack->setCurrentIndex(DEFAULT_VIEW); // по умолчанию — view A
	//group 1: four view buttons
	m_view_group = new QButtonGroup(this);
	m_view_group->setExclusive(true); // only one active

	for(int i = 1; i <= 4; ++i)
	{
		QToolButton* btn = new QToolButton(this);
		btn->setIcon(make_view_icon(i - 1));
		//btn->setText(QString::number(i));
		btn->setCheckable(true);
		btn->setAutoRaise(true);
		btn->setFixedSize(24, 24);

		m_view_group->addButton(btn, i); // add with id = i
		m_view_buttons.append(btn);
	}
	m_view_buttons[0]->setToolTip(tr("View A"));
	m_view_buttons[1]->setToolTip(tr("View B"));
	m_view_buttons[2]->setToolTip(tr("Split"));
	m_view_buttons[3]->setToolTip(tr("Intensity"));
	m_view_buttons[DEFAULT_VIEW]->setChecked(true); // default button
	//group 2: parameters
	m_current_label_group = new QGroupBox(tr("Parameters"), this);

	m_current_label = new QLabel(m_current_label_group);
	m_current_label->setTextFormat(Qt::PlainText); // no HTML interpretation
	m_current_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_center_of_gravity_text = QString("COG A: --\n"
									   "COG B: --");

	auto* current_label_layout = new QVBoxLayout(m_current_label_group);
	current_label_layout->addWidget(m_current_label);

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
	side_layout->addWidget(m_current_label_group);

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

	//calls for histogram plot
	connect(m_capacitor_combo_box, qOverload<int>(&QComboBox::currentIndexChanged), this,
			&BeamPanel::apply_current_scale_to_plots);

	connect(m_conversion_spin_box, qOverload<int>(&QSpinBox::valueChanged), this,
			&BeamPanel::apply_current_scale_to_plots);
	//hover mouse in histogram plot lambda
	// ... existing connects ...

	connect(m_hist_a, &HistogramPlot::bar_hovered, this,
			[this](int a_index, int, double)
			{
				m_hover_source = HoverSource::HistA;
				m_hover_index = a_index;
				refresh_label();
			});

	connect(m_hist_b, &HistogramPlot::bar_hovered, this,
			[this](int a_index, int, double)
			{
				m_hover_source = HoverSource::HistB;
				m_hover_index = a_index;
				refresh_label();
			});

	connect(m_hist_a, &HistogramPlot::bar_unhovered, this,
			[this]
			{
				if(m_hover_source == HoverSource::HistA)
				{
					m_hover_source = HoverSource::None;
					m_hover_index = -1;
					refresh_label();
				}
			});

	connect(m_hist_b, &HistogramPlot::bar_unhovered, this,
			[this]
			{
				if(m_hover_source == HoverSource::HistB)
				{
					m_hover_source = HoverSource::None;
					m_hover_index = -1;
					refresh_label();
				}
			});

	connect(m_heatmap, &HeatmapPlot::cell_hovered, this,
			[this](int a_ix, int a_iy, int, int, double)
			{
				m_hover_source = HoverSource::Heatmap;
				m_hover_ix = a_ix;
				m_hover_iy = a_iy;
				refresh_label();
			});

	connect(m_heatmap, &HeatmapPlot::cell_unhovered, this,
			[this]
			{
				if(m_hover_source == HoverSource::Heatmap)
				{
					m_hover_source = HoverSource::None;
					m_hover_ix = -1;
					m_hover_iy = -1;
					refresh_label();
				}
			});
}

void BeamPanel::on_view_changed(int a_id)
{
	switch(a_id)
	{
		case 1:
			m_view_mode = ViewMode::Bars;
			m_stack->setCurrentIndex(0);
			m_hist_a->show();
			m_hist_b->hide();
			break;
		case 2:
			m_view_mode = ViewMode::Bars;
			m_stack->setCurrentIndex(0);
			m_hist_a->hide();
			m_hist_b->show();
			break;
		case 3:
			m_view_mode = ViewMode::Bars;
			m_stack->setCurrentIndex(0);
			m_hist_a->show();
			m_hist_b->show();
			break;
		case 4:
			m_view_mode = ViewMode::Heatmap;
			m_stack->setCurrentIndex(1);
			break;
	}

	// hover from the previous mode is no longer valid
	m_hover_source = HoverSource::None;
	m_hover_index = -1;
	m_hover_ix = -1;
	m_hover_iy = -1;

	refresh_label();
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

void BeamPanel::set_center_of_gravity(double a_value_a, double a_value_b)
{
	m_center_of_gravity_text = QString("COG A: %1\n"
									   "COG B: %2")
								   .arg(a_value_a, 0, 'f', 3)
								   .arg(a_value_b, 0, 'f', 3);
	// no setText here — caller decides
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

void BeamPanel::on_histograms_ready(const Histograms& a_hist)
{
	if(!a_hist.a.isEmpty())
		m_data_a = a_hist.a;
	if(!a_hist.b.isEmpty())
		m_data_b = a_hist.b;
	Q_ASSERT(m_data_a.size() == m_data_b.size());

	const int n = m_data_a.size();

	//TODO: add extra value in array
	set_center_of_gravity(m_data_a[n - 1], m_data_b[n - 1]);
	update_plots();
	// single label refresh after all data is updated
	refresh_label();
}

void BeamPanel::update_plots()
{
	m_hist_a->setData(m_data_a);
	m_hist_b->setData(m_data_b);
	m_heatmap->setData(m_data_a, m_data_b);
}

void BeamPanel::apply_current_scale_to_plots()
{
	const auto p = parameters();

	const double charge_pC = charge_from_capacitor(p.capacitor);
	const double conv_us = p.conversion_us;

	m_hist_a->setCurrentScale(charge_pC, conv_us);
	m_hist_b->setCurrentScale(charge_pC, conv_us);
	// refresh the label so the currents reflect the new scale
	refresh_label();
}

double BeamPanel::charge_from_capacitor(Capacitor a_cap) const
{
	switch(a_cap)
	{
		case Capacitor::C12p5:
			return 12.5;
		case Capacitor::C50:
			return 50.0;
		case Capacitor::C100:
			return 100.0;
		case Capacitor::C150:
			return 150.0;
	}
	return 0.0;
}

QString BeamPanel::build_bars_text() const
{
	const auto p = parameters();
	const double Q = charge_from_capacitor(p.capacitor);
	const double t = p.conversion_us;

	// ─── currents ────────────────────────────────────────────────
	QString current_lines;
	if(t <= 0.0)
	{
		current_lines = "I_A:   --\nI_B:   --\nI_sum: --";
	}
	else
	{
		const double k = (Q / t) * 1000.0 / (65535.0 - ADC_OFFSET_LSB);

		double sum_a = 0.0, sum_b = 0.0;
		for(int v: m_data_a)
			sum_a += (v - ADC_OFFSET_LSB) * k;
		for(int v: m_data_b)
			sum_b += (v - ADC_OFFSET_LSB) * k;

		current_lines = QString("I_A:   %1 nA\n"
								"I_B:   %2 nA\n"
								"I_sum: %3 nA")
							.arg(sum_a, 0, 'f', 3)
							.arg(sum_b, 0, 'f', 3)
							.arg(sum_a + sum_b, 0, 'f', 3);
	}

	// ─── hover: recompute from fresh data ───────────────────────
	QString hover_lines;

	if((m_hover_source == HoverSource::HistA || m_hover_source == HoverSource::HistB) &&
	   m_hover_index >= 0)
	{
		const QVector<int>& src = (m_hover_source == HoverSource::HistA) ? m_data_a : m_data_b;

		if(m_hover_index < src.size())
		{
			const int adc = src[m_hover_index];
			const double k = (t > 0.0) ? (Q / t) * 1000.0 / (65535.0 - ADC_OFFSET_LSB) : 0.0;
			const double current = (adc - ADC_OFFSET_LSB) * k;

			hover_lines = QString("Position: %1\n"
								  "ADC:      %2\n"
								  "Current:  %3 nA")
							  .arg(m_hover_index)
							  .arg(adc)
							  .arg(current, 0, 'f', 3);
		}
	}

	if(hover_lines.isEmpty())
	{
		hover_lines = "Position: --\n"
					  "ADC:      --\n"
					  "Current:  -- nA";
	}

	return current_lines + "\n\n" + hover_lines + "\n\n" + m_center_of_gravity_text;
}

QString BeamPanel::build_heatmap_text() const
{
	QString hover_lines;
	QString intensity_line;

	if(m_hover_source == HoverSource::Heatmap && m_hover_ix >= 0 && m_hover_ix < m_data_a.size() &&
	   m_hover_iy >= 0 && m_hover_iy < m_data_b.size())
	{
		const int adc_x = m_data_a[m_hover_ix];
		const int adc_y = m_data_b[m_hover_iy];

		const int max_x =
			m_data_a.isEmpty() ? 0 : *std::max_element(m_data_a.begin(), m_data_a.end());
		const int max_y =
			m_data_b.isEmpty() ? 0 : *std::max_element(m_data_b.begin(), m_data_b.end());
		const double norm = std::max(1.0, double(max_x) * double(max_y));
		const double intensity = double(adc_x) * double(adc_y) / norm;

		hover_lines = QString("X:         ch %1\n"
							  "Y:         ch %2\n"
							  "ADC_X:     %3\n"
							  "ADC_Y:     %4")
						  .arg(m_hover_ix)
						  .arg(m_hover_iy)
						  .arg(adc_x)
						  .arg(adc_y);

		intensity_line = QString("Intensity: %1").arg(intensity, 0, 'f', 3);
	}
	else
	{
		hover_lines = "X:         --\n"
					  "Y:         --\n"
					  "ADC_X:     --\n"
					  "ADC_Y:     --";
		intensity_line = "Intensity: --";
	}

	return hover_lines + "\n\n" + intensity_line + "\n\n" + m_center_of_gravity_text;
}

void BeamPanel::refresh_label()
{
	m_current_label->setText(m_view_mode == ViewMode::Bars ? build_bars_text()
														   : build_heatmap_text());
}