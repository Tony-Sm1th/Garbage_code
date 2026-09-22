// data_emulator.cpp
#include "data_emulator.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QDebug>

DataEmulator::DataEmulator(QObject* parent) : QObject(parent)
{
	connect(&m_stream_timer, &QTimer::timeout, this, &DataEmulator::on_stream_tick);
}

// ─── commands from BeamPanel ─────────────────────────────────────

void DataEmulator::on_single_read_requested()
{
	qDebug() << "emulator: single read";

	// generate one frame and emit it immediately
	emit histograms_ready(generate_one_frame());
}

void DataEmulator::on_continuous_read_requested(int a_interval_ms)
{
	qDebug() << "emulator: start continuous read, interval" << a_interval_ms << "ms";

	m_stream_timer.start(a_interval_ms);
}

void DataEmulator::on_continuous_read_stopped()
{
	qDebug() << "emulator: stop continuous read";

	m_stream_timer.stop();
}

// ─── periodic tick while streaming ──────────────────────────────

void DataEmulator::on_stream_tick()
{
	emit histograms_ready(generate_one_frame());
}

// ─── data generation ────────────────────────────────────────────

BeamPanel::Histograms DataEmulator::generate_one_frame()
{
	++m_tick_counter;
	const double phase = m_tick_counter * 0.1;

	const double center_a = 64.0 + 4.0 * std::sin(phase);
	const double center_b = 64.0 + 4.0 * std::cos(phase);

	const double amp_a = 65000.0 + 2000.0 * std::sin(phase * 1.3);
	const double amp_b = 25000.0 + 2000.0 * std::cos(phase * 1.7);

	BeamPanel::Histograms h;
	h.a = make_profile(center_a, 10.0, amp_a, 128);
	h.b = make_profile(center_b, 12.0, amp_b, 128);

	return h;
}

QVector<int> DataEmulator::make_profile(double a_center, double a_sigma, double a_amplitude,
										int a_channels) const
{
	QVector<int> profile(a_channels);
	auto* rng = QRandomGenerator::global();

	for(int i = 0; i < a_channels; ++i)
	{
		const double dx = (i - a_center) / a_sigma;
		const double base = a_amplitude * std::exp(-0.5 * dx * dx);
		const double noise = (rng->generateDouble() - 0.5) * 0.04 * a_amplitude;

		const double value = base + noise;
		profile[i] = qBound(0, int(value), 65535);
	}

	return profile;
}