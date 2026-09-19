#pragma once

#include <QWidget>
#include <QList>

class QTextEdit;
class QPushButton;
class QToolButton;
class QButtonGroup;

class beam_panel : public QWidget
{
	Q_OBJECT

  private:
	QTextEdit* m_histogram_plot = nullptr;
	QPushButton* m_random_button = nullptr;
	QList<QToolButton*> m_view_buttons;
	QButtonGroup* m_view_group = nullptr;

	void build_ui();
	void build_layout();
	void connect_signals();

  private slots:
	void on_view_changed(int a_id);

  public:
	explicit beam_panel(QWidget* a_parent = nullptr);
	~beam_panel() = default;
};