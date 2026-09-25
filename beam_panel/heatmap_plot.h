#pragma once

#include "qcustomplot.h"
#include <QVector>

class QCPColorMap;
class QCPColorScale;

class HeatmapPlot : public QCustomPlot
{
	Q_OBJECT

  public:
	explicit HeatmapPlot(QWidget* parent = nullptr);

	void setData(const QVector<int>& a_x_values, const QVector<int>& a_y_values);

	void setTitle(const QString& a_title);

  signals:
	// emitted when the mouse hovers a cell
	void cell_hovered(int a_ix, int a_iy, int a_adc_x, int a_adc_y, double a_intensity);
	void cell_unhovered();

  protected:
	void mouseMoveEvent(QMouseEvent* a_event) override;
	void leaveEvent(QEvent* a_event) override;
	void resizeEvent(QResizeEvent* a_event) override;
	void showEvent(QShowEvent* a_event) override;

  private:
	QCPAxisRect* m_axis_rect = nullptr;
	QCPAxis* m_axis_x = nullptr;
	QCPAxis* m_axis_y = nullptr;

	QCPColorMap* m_color_map = nullptr;
	QCPColorScale* m_color_scale = nullptr;

	QVector<int> m_values_x;
	QVector<int> m_values_y;

	double m_norm = 1.0; // max(A) * max(B), for normalization
	int m_hovered_index = -1;

	void setup_axes();
	void setup_color_map();
	void rebuild();
	void update_square_axis_rect();
};