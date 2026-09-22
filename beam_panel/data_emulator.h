// data_emulator.h
#pragma once

#include <QObject>
#include <QTimer>
#include "beam_panel.h"

class DataEmulator : public QObject
{
	Q_OBJECT

  public:
	explicit DataEmulator(QObject* parent = nullptr);

  public slots:
	// commands coming from BeamPanel — same as a real controller would receive
	void on_single_read_requested();
	void on_continuous_read_requested(int a_interval_ms);
	void on_continuous_read_stopped();

  signals:
	// data going back to BeamPanel — same as a real controller would emit
	void histograms_ready(const BeamPanel::Histograms& a_hist);

  private slots:
	void on_stream_tick();

  private:
	QVector<int> make_profile(double a_center, double a_sigma, double a_amplitude,
							  int a_channels) const;

	BeamPanel::Histograms generate_one_frame();

	QTimer m_stream_timer;
	int m_tick_counter = 0;
};