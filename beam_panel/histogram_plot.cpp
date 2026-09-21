#include "histogram_plot.h"

#include <algorithm> // std::max_element

HistogramPlot::HistogramPlot(QWidget* parent) : QCustomPlot(parent)
{
	setup_axes();
	setup_bars();

	setMinimumHeight(150);
	setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void HistogramPlot::setup_axes()
{
	m_axis_rect = axisRect();

	m_axis_x = m_axis_rect->axis(QCPAxis::atBottom);
	m_axis_y = m_axis_rect->axis(QCPAxis::atLeft);

	m_axis_x->setLabel(tr("Channel"));
	m_axis_y->setLabel(tr("ADC"));

	m_axis_x->setRange(0, 128);
	m_axis_y->setRange(0, 100);

	// hide ticks and labels on X for now
	m_axis_x->setTickLabels(false);
}

void HistogramPlot::setup_bars()
{
	m_bars = new QCPBars(m_axis_x, m_axis_y);

	// bar width in X-axis units
	m_bars->setWidth(0.8);
	m_bars->setPen(Qt::NoPen);
	m_bars->setBrush(QColor(70, 130, 180));
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

	replot();
}