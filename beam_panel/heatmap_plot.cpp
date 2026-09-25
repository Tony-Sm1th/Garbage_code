#include "heatmap_plot.h"
#include <algorithm> // std::max, std::max_element
#include <cmath>	 // std::floor
#include <QMouseEvent>

#define X_POS_OFFSET 1 //start x index from 1 on mouse hover
#define Y_POS_OFFSET 1 //start y index from 1 on mouse hover

HeatmapPlot::HeatmapPlot(QWidget* parent) : QCustomPlot(parent)
{
	setup_axes();
	setup_color_map();

	setMinimumHeight(150);
	setMinimumSize(150, 150);
	// initialize with zeros
	m_values_x.fill(0, 128);
	m_values_y.fill(0, 128);
	rebuild();
	update_square_axis_rect();
}

// ─────────────────────────────────────────────────────────────────
// setup
// ─────────────────────────────────────────────────────────────────

void HeatmapPlot::setup_axes()
{
	m_axis_rect = axisRect();

	m_axis_x = m_axis_rect->axis(QCPAxis::atBottom);
	m_axis_y = m_axis_rect->axis(QCPAxis::atLeft);

	m_axis_x->setLabel(tr("A channel"));
	m_axis_y->setLabel(tr("B channel"));

	m_axis_x->setRange(0, 128);
	m_axis_y->setRange(0, 128);
}

void HeatmapPlot::setup_color_map()
{
	m_color_map = new QCPColorMap(m_axis_x, m_axis_y);
	m_color_map->setInterpolate(true);

	m_color_scale = new QCPColorScale(this);
	plotLayout()->addElement(0, 1, m_color_scale); // right of the plot
	m_color_map->setColorScale(m_color_scale);

	// blue -> cyan -> yellow -> red
	m_color_map->setGradient(QCPColorGradient::gpJet);

	// fixed range: intensity is always in [0, 1]
	m_color_map->setDataRange(QCPRange(0.0, 1.0));
}

// ─────────────────────────────────────────────────────────────────
// data
// ─────────────────────────────────────────────────────────────────

void HeatmapPlot::setData(const QVector<int>& a_x_values, const QVector<int>& a_y_values)
{
	m_values_x = a_x_values;
	m_values_y = a_y_values;
	rebuild();
}

void HeatmapPlot::setTitle(const QString& a_title)
{
	plotLayout()->insertRow(0);
	plotLayout()->addElement(0, 0, new QCPTextElement(this, a_title));
}

// ─────────────────────────────────────────────────────────────────
// rebuild
// ─────────────────────────────────────────────────────────────────

void HeatmapPlot::rebuild()
{
	const int nx = m_values_x.size();
	const int ny = m_values_y.size();
	if(nx == 0 || ny == 0)
		return;

	m_color_map->data()->setSize(nx, ny);
	m_color_map->data()->setRange(QCPRange(0, nx), QCPRange(0, ny));

	// normalization: intensity = A[x] * B[y] / (max(A) * max(B))
	// so the maximum intensity is always 1.0
	const int max_x =
		m_values_x.isEmpty() ? 0 : *std::max_element(m_values_x.begin(), m_values_x.end());
	const int max_y =
		m_values_y.isEmpty() ? 0 : *std::max_element(m_values_y.begin(), m_values_y.end());

	m_norm = double(max_x) * double(max_y);
	if(m_norm <= 0.0)
		m_norm = 1.0;

	for(int x = 0; x < nx; ++x)
	{
		for(int y = 0; y < ny; ++y)
		{
			const double value = double(m_values_x[x]) * double(m_values_y[y]) / m_norm;
			m_color_map->data()->setCell(x, y, value);
		}
	}

	m_axis_x->setRange(0, nx);
	m_axis_y->setRange(0, ny);

	// color range is always [0, 1]
	m_color_map->setDataRange(QCPRange(0.0, 1.0));

	replot();
}

// ─────────────────────────────────────────────────────────────────
// mouse handling
// ─────────────────────────────────────────────────────────────────

void HeatmapPlot::mouseMoveEvent(QMouseEvent* a_event)
{
	QCustomPlot::mouseMoveEvent(a_event);

	if(m_values_x.isEmpty() || m_values_y.isEmpty())
		return;

	const double x_data = m_axis_x->pixelToCoord(a_event->pos().x());
	const double y_data = m_axis_y->pixelToCoord(a_event->pos().y());

	const int ix = int(std::floor(x_data));
	const int iy = int(std::floor(y_data));

	// out of range: unhover
	if(ix < 0 || ix >= m_values_x.size() || iy < 0 || iy >= m_values_y.size())
	{
		if(m_hovered_index != -1)
		{
			m_hovered_index = -1;
			emit cell_unhovered();
		}
		return;
	}

	const int index = iy * m_values_x.size() + ix;
	if(index == m_hovered_index)
		return;

	m_hovered_index = index;

	const int adc_x = m_values_x[ix];
	const int adc_y = m_values_y[iy];
	const double intensity = double(adc_x) * double(adc_y) / m_norm;

	emit cell_hovered(ix + X_POS_OFFSET, iy + Y_POS_OFFSET, adc_x, adc_y, intensity);
}

void HeatmapPlot::leaveEvent(QEvent* a_event)
{
	QCustomPlot::leaveEvent(a_event);

	if(m_hovered_index != -1)
	{
		m_hovered_index = -1;
		emit cell_unhovered();
	}
}

void HeatmapPlot::resizeEvent(QResizeEvent* a_event)
{
	QCustomPlot::resizeEvent(a_event);
	update_square_axis_rect();
}

void HeatmapPlot::showEvent(QShowEvent* a_event)
{
	QCustomPlot::showEvent(a_event);
	update_square_axis_rect(); // layout once the widget is really shown
}

void HeatmapPlot::update_square_axis_rect()
{
	const int w = width();
	const int h = height();

	// reserve space for labels, title and color scale
	const int margin_left = 50;	  // Y axis labels
	const int margin_bottom = 40; // X axis labels
	const int margin_top = 30;	  // title
	const int margin_right = 80;  // color scale

	const int avail_w = w - margin_left - margin_right;
	const int avail_h = h - margin_top - margin_bottom;

	const int min_side = 100;
	if(avail_w < min_side || avail_h < min_side)
		return;

	const int side = qMin(avail_w, avail_h);

	const int left = margin_left + (avail_w - side) / 2;
	const int top = margin_top + (avail_h - side) / 2;
	const int right = w - left - side;
	const int bottom = h - top - side;

	// disable auto margins, then set manual ones
	m_axis_rect->setAutoMargins(QCP::msNone);
	m_axis_rect->setMargins(QMargins(left, top, right, bottom));

	replot();
}