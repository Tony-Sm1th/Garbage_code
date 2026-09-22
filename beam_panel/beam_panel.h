#pragma once

#define DEFAULT_CONV_VALUE 20000		  // us
#define DEFAULT_CONV_MIN_RANGE 160		  // us
#define DEFAULT_CONV_MAX_RANGE 2000000000 // us

#define DEFAULT_LEVEL_VALUE 500		  // parrots
#define DEFAULT_LEVEL_MIN_RANGE 1	  // parrots
#define DEFAULT_LEVEL_MAX_RANGE 65535 // parrots

#define DEFAULT_COMMANDS_VALUE 1000		   // ms
#define DEFAULT_COMMANDS_MIN_RANGE 1	   // ms
#define DEFAULT_COMMANDS_MAX_RANGE 2000000 // ms

#define ADC_OFFSET_LSB 250

#include <QWidget>
#include <QList>
#include <QVector>
#include <QMetaType>

class QTextEdit;
class QPushButton;
class QToolButton;
class QButtonGroup;
class QGroupBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QStackedWidget;
class QSplitter;

class HistogramPlot;

class BeamPanel : public QWidget
{
	Q_OBJECT

  public:
	enum class Capacitor
	{
		C12p5 = 0,
		C50,
		C100,
		C150
	};
	Q_ENUM(Capacitor)

	enum class TriggerMode
	{
		Hardware = 0,
		Software
	};
	Q_ENUM(TriggerMode)

	struct BeamParameters
	{
		Capacitor capacitor = Capacitor::C50;
		int conversion_us = DEFAULT_CONV_VALUE;
		TriggerMode trigger_mode = TriggerMode::Hardware;
		bool trigger_pulse = false;
		int trigger_level_lsb = DEFAULT_LEVEL_VALUE;
		bool noise_cancellation = false;
		int read_interval_ms = DEFAULT_COMMANDS_VALUE;
	};

	struct Histograms
	{
		QVector<int> a;
		QVector<int> b;
	};

	explicit BeamPanel(QWidget* a_parent = nullptr);
	~BeamPanel() override = default;

	BeamParameters parameters() const;

  signals:
	void parameters_ready(const BeamPanel::BeamParameters& a_params);
	void single_read_requested();
	void continuous_read_requested(int a_interval_ms);
	void continuous_read_stopped();
	void noise_accumulation_requested();

  public slots:
	void on_histograms_ready(const BeamPanel::Histograms& a_hist);
	// void on_parameters_applied(const BeamPanel::BeamParameters& a_params);
	// void on_error(const QString& a_message);

  private slots:
	void on_view_changed(int a_id);
	void on_noise_background_clicked();
	void on_send_parameters_clicked();
	void on_single_read_clicked();
	void on_continuous_read_toggled(bool a_on);
	void apply_current_scale_to_plots();

  private:
	QStackedWidget* m_stack = nullptr;

	HistogramPlot* m_hist_a = nullptr; // page 0
	HistogramPlot* m_hist_b = nullptr; // page 1
	QSplitter* m_view_split = nullptr; // page 2
	HistogramPlot* m_hist_split_a = nullptr;
	HistogramPlot* m_hist_split_b = nullptr;
	HistogramPlot* m_hist_sum = nullptr; // page 3

	QVector<int> m_data_a;
	QVector<int> m_data_b;
	QVector<int> m_data_sum;

	QList<QToolButton*> m_view_buttons;
	QButtonGroup* m_view_group = nullptr;

	double m_cog_value = 0.0;
	QGroupBox* m_center_of_gravity_group = nullptr;
	QLabel* m_center_of_gravity_label = nullptr;
	QLabel* m_current_label = nullptr;

	QGroupBox* m_capacitor_group = nullptr;
	QComboBox* m_capacitor_combo_box = nullptr;

	QGroupBox* m_conversion_group = nullptr;
	QLabel* m_conversion_label = nullptr;
	QSpinBox* m_conversion_spin_box = nullptr;

	QGroupBox* m_trigger_group = nullptr;
	QLabel* m_mode_trigger_label = nullptr;
	QComboBox* m_mode_trigger_combo_box = nullptr;
	QCheckBox* m_mode_trigger_check_box = nullptr;
	QLabel* m_mode_trigger_level_label = nullptr;
	QSpinBox* m_mode_trigger_level_spin_box = nullptr;

	QGroupBox* m_noise_background_group = nullptr;
	QPushButton* m_noise_background_btn = nullptr;
	QCheckBox* m_noise_background_check_box = nullptr;

	QGroupBox* m_parameters_send_group = nullptr;
	QPushButton* m_parameters_send_btn = nullptr;

	QGroupBox* m_reading_group = nullptr;
	QPushButton* m_reading_single_read_btn = nullptr;
	QLabel* m_reading_continuous_read_label = nullptr;
	QPushButton* m_reading_continuous_read_btn = nullptr;
	QSpinBox* m_reading_spin_box = nullptr;

	void build_ui();
	void build_layout();
	void connect_signals();
	void set_center_of_gravity(double a_value);
	void update_histograms();
	void update_parameter_labels();
	//for histogram current axis
	double charge_from_capacitor(Capacitor a_cap);
};

Q_DECLARE_METATYPE(BeamPanel::Histograms)
