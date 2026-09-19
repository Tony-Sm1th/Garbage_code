#pragma once
#include <QWidget>

class QTabWidget;
class MainTab;
class BeamPanel;

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() = default;

private:
    void buildUi();
    void buildLayout();
    void connectSignals();

    QTabWidget* m_tabs    = nullptr;
    MainTab*    m_mainTab = nullptr;
    BeamPanel* m_beamPanelTab = nullptr;
};