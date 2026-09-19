#pragma once
#include <QWidget>

class QTextEdit;
class QPushButton;

class BeamPanel: public QWidget
{
    Q_OBJECT
public:
    explicit BeamPanel(QWidget *parent = nullptr);
    ~BeamPanel() = default;

private:
    void buildUi();
    void buildLayout();
    void connectSignals();
    QTextEdit*   m_histogramPlot     = nullptr;
    QPushButton* m_randomButton = nullptr;
};