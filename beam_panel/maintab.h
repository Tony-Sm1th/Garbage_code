#pragma once
#include <QWidget>

class QTextEdit;
class QPushButton;

class MainTab : public QWidget
{
    Q_OBJECT
public:
    explicit MainTab(QWidget *parent = nullptr);
    ~MainTab() = default;

private:
    QTextEdit*   m_textEdit     = nullptr;
    QPushButton* m_randomButton = nullptr;
};