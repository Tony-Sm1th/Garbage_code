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

#include <QWidget>
#include <QList>

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

class BeamPanel : public QWidget
{
	Q_OBJECT

  private:
	//left side of the widget
	// left side
	QStackedWidget* m_stack = nullptr;
	QTextEdit* m_view_a = nullptr;	 // page 0
	QTextEdit* m_view_b = nullptr;	 // page 1
	QWidget* m_view_split = nullptr; // page 2
	QTextEdit* m_view_sum = nullptr; // page 3

	//right side of the widget
	//group 1
	QList<QToolButton*> m_view_buttons;
	QButtonGroup* m_view_group = nullptr;

	//group 2
	double m_cog_value = 0.0;
	QGroupBox* m_center_of_gravity_group = nullptr;
	QLabel* m_center_of_gravity_label = nullptr;

	//group 3
	QGroupBox* m_capacitor_group = nullptr;
	QComboBox* m_capacitor_combo_box = nullptr;

	//group 4
	QGroupBox* m_conversion_group = nullptr;
	QLabel* m_conversion_label = nullptr;
	QSpinBox* m_conversion_spin_box = nullptr;

	//group 5
	QGroupBox* m_trigger_group = nullptr;
	QLabel* m_mode_trigger_label = nullptr;
	QComboBox* m_mode_trigger_combo_box = nullptr;

	QCheckBox* m_mode_trigger_check_box = nullptr;

	QLabel* m_mode_trigger_level_label = nullptr;
	QSpinBox* m_mode_trigger_level_spin_box = nullptr;

	//group 6
	QGroupBox* m_noise_background_group = nullptr;
	QPushButton* m_noise_background_btn = nullptr;
	QCheckBox* m_noise_background_check_box = nullptr;

	//group 7
	QGroupBox* m_commands_group = nullptr;
	QPushButton* m_commands_send_btn = nullptr;
	QPushButton* m_commands_single_read_btn = nullptr;
	QLabel* m_commands_continuous_read_label = nullptr;
	QPushButton* m_commands_continuous_read_btn = nullptr;
	QSpinBox* m_commands_spin_box = nullptr;

	void build_ui();
	void build_layout();
	void connect_signals();
	//methods
	void set_center_of_gravity(double a_value);

  private slots:
	void on_view_changed(int a_id);
	void on_trigger_mode_changed(int index);
	void on_trigger_mode_pulse_checked(bool is_checked);
	void on_noise_background_clicked();
	void on_commands_continuous_read_toggled(bool is_toggled);

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

	explicit BeamPanel(QWidget* a_parent = nullptr);
	~BeamPanel() = default;
};