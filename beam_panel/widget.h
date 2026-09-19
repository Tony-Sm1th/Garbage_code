#pragma once
#include <QWidget>

class QTabWidget;
class MainTab;
class beam_panel;

class Widget : public QWidget
{
	Q_OBJECT
  public:
	explicit Widget(QWidget* parent = nullptr);
	~Widget() = default;

  private:
	void buildUi();
	void buildLayout();
	void connectSignals();

	QTabWidget* m_tabs = nullptr;
	MainTab* m_mainTab = nullptr;
	beam_panel* m_beam_panel_tab = nullptr;
};