#include "histogram_plot.h"

#include <algorithm> // std::max_element

HistogramPlot::HistogramPlot(QWidget* parent) : QCustomPlot(parent)
{
	setup_axes();
	setup_bars();

	setMinimumHeight(150);
	//setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void HistogramPlot::setup_axes()
{
	m_axis_rect = axisRect();

	m_axis_x = m_axis_rect->axis(QCPAxis::atBottom);
	m_axis_y = m_axis_rect->axis(QCPAxis::atLeft);

	m_axis_x->setLabel(tr("Channel"));
	m_axis_y->setLabel(tr("ADC"));

	m_axis_x->setRange(0, 128);
	m_axis_y->setRange(0, 65535);

	m_axis_right = m_axis_rect->addAxis(QCPAxis::atRight);
	m_axis_right->setLabel(tr("Current, nA"));
	// hide ticks and labels on X for now
	//m_axis_x->setTickLabels(false);
	auto ticker = QSharedPointer<QCPAxisTickerFixed>::create();
	ticker->setTickStep(16.0);
	ticker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
	m_axis_x->setTicker(ticker);
	m_axis_x->setTickLabels(true);
}

void HistogramPlot::setup_bars()
{
	m_bars = new QCPBars(m_axis_x, m_axis_y);

	// Bar width
	m_bars->setWidth(0.8);

	// Normal bars — clean medium blue
	m_bars->setPen(Qt::NoPen);
	m_bars->setBrush(QColor(65, 130, 190)); // #4182BE

	// Single-bar selection
	m_bars->setSelectable(QCP::stSingleData);

	// Selection style
	QCPSelectionDecorator* deco = m_bars->selectionDecorator();

	deco->setPen(QPen(QColor(248, 138, 29), 2.5)); // rgb(248, 138, 29)
	deco->setBrush(QBrush(QColor(255, 179, 26)));  // rgb(255, 179, 26)
}

void HistogramPlot::setData(const QVector<int>& a_values)
{
	m_values = a_values;
	rebuild();
}

void HistogramPlot::setTitle(const QString& a_title)
{
	// insert a new row at the top of the plot layout and put a text element there
	plotLayout()->insertRow(0);
	plotLayout()->addElement(0, 0, new QCPTextElement(this, a_title));
}

void HistogramPlot::setBarColor(const QColor& a_color)
{
	m_bars->setBrush(a_color);
	replot();
}

void HistogramPlot::rebuild()
{
	QVector<double> keys;
	QVector<double> values;
	keys.reserve(m_values.size());
	values.reserve(m_values.size());

	for(int i = 0; i < m_values.size(); ++i)
	{
		// bar center is at i + 0.5
		keys.append(i + 0.5);
		values.append(m_values[i]);
	}

	m_bars->setData(keys, values);

	// X range always covers all channels
	m_axis_x->setRange(0, m_values.size());

	// Y range: auto-scale to the maximum value with 10% headroom
	if(!values.isEmpty())
	{
		const double max_value = *std::max_element(values.begin(), values.end());
		m_axis_y->setRange(0, max_value * 1.1);
	}
	m_axis_right->setRange(m_axis_y->range());
	update_right_axis_labels();

	replot();
}

void HistogramPlot::update_right_axis_labels()
{
	auto ticker = QSharedPointer<QCPAxisTickerText>::create();

	const QCPRange range = m_axis_y->range();
	const int tick_count = 6;
	const double step = (range.upper - range.lower) / (tick_count - 1);

	for(int i = 0; i < tick_count; ++i)
	{
		const double adc = range.lower + step * i;
		const double current_nA = (adc - ADC_OFFSET_LSB) * m_current_k;
		ticker->addTick(adc, QString::number(current_nA, 'f', 3).rightJustified(11, ' '));
	}

	m_axis_right->setTicker(ticker);
}

void HistogramPlot::setCurrentScale(double a_charge_pC, double a_conversion_us)
{
	if(a_conversion_us <= 0.0)
	{
		m_current_k = 0.0;
		return;
	}

	// I_max [тA] = Q [pC] / t [us] * 1000
	const double i_max_nA = a_charge_pC / a_conversion_us * 1000;

	// ADC range that corresponds to [0 .. I_max] is (65535 - offset)
	const double adc_span = 65535.0 - double(ADC_OFFSET_LSB);

	m_current_k = i_max_nA / adc_span;

	// if you show the right axis, update its labels here
	update_right_axis_labels();

	replot();
}

void HistogramPlot::mouseMoveEvent(QMouseEvent* a_event)
{
	QCustomPlot::mouseMoveEvent(a_event);

	if(m_values.isEmpty())
		return;

	const double x_data = m_axis_x->pixelToCoord(a_event->pos().x());
	const int index = int(std::floor(x_data));

	// out of range: unhover
	if(index < 0 || index >= m_values.size())
	{
		if(m_hovered_index != -1)
		{
			m_hovered_index = -1;
			m_bars->setSelection(QCPDataSelection());
			replot();
			emit bar_unhovered();
		}
		return;
	}

	// same index: nothing to do
	if(index == m_hovered_index)
		return;

	// new hovered index
	m_hovered_index = index;

	// highlight via selection
	m_bars->setSelection(QCPDataSelection(QCPDataRange(index, index + 1)));

	// compute values
	const int adc = m_values[index];
	const double current_nA = (adc - ADC_OFFSET_LSB) * m_current_k;

	emit bar_hovered(index + X_POS_OFFSET, adc, current_nA);

	replot();
}

void HistogramPlot::leaveEvent(QEvent* a_event)
{
	QCustomPlot::leaveEvent(a_event);

	if(m_hovered_index != -1)
	{
		m_hovered_index = -1;
		m_bars->setSelection(QCPDataSelection());
		replot();
		emit bar_unhovered();
	}
}