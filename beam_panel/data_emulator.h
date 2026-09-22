#pragma once

#include <QObject>
#include <QTimer>
#include "beam_panel.h" // for BeamPanel::Histograms

class DataEmulator : public QObject
{
	Q_OBJECT

  public:
	explicit DataEmulator(QObject* parent = nullptr);

	void start(int a_interval_ms = 1000);
	void stop();

  signals:
	void histograms_ready(const BeamPanel::Histograms& a_hist);

  private slots:
	void on_tick();

  private:
	QTimer m_timer;
	int m_tick_counter = 0;

	QVector<int> make_profile(double a_center, double a_sigma, double a_amplitude,
							  int a_channels) const;
};