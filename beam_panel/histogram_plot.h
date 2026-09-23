#pragma once
#define ADC_OFFSET_LSB 250
#define X_POS_OFFSET 1 //start index from 1 on mouse hover

#include "qcustomplot.h"
#include <QVector>

class HistogramPlot : public QCustomPlot
{
	Q_OBJECT

  public:
	explicit HistogramPlot(QWidget* parent = nullptr);

	void setData(const QVector<int>& a_values);
	void setTitle(const QString& a_title);
	void setBarColor(const QColor& a_color);
	void setCurrentScale(double a_charge_pC, double a_conversion_us);

  signals:
	//mouse bar hover
	void bar_hovered(int a_index, int a_adc, double a_current_nA);
	void bar_unhovered();

  protected:
	//mouse bar hover
	void mouseMoveEvent(QMouseEvent* a_event) override;
	void leaveEvent(QEvent* a_event) override;

  private:
	QCPBars* m_bars = nullptr;			// bar series (the histogram itself)
	QCPAxisRect* m_axis_rect = nullptr; // axis rectangle (the plot area)
	QCPAxis* m_axis_x = nullptr;		// bottom axis (X)
	QCPAxis* m_axis_y = nullptr;		// left axis (Y)
	QCPAxis* m_axis_right = nullptr;	// right axis for current (nA)

	QVector<int> m_values;

	double m_current_k = 0.0; // current per ADC count, in chosen units
	//mouse bar hover
	int m_hovered_index = -1;

	void setup_axes();
	void setup_bars();
	void update_right_axis_labels();
	void rebuild();
};