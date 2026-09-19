#include "beam_panel.h"
#include <QtWidgets>

BeamPanel::BeamPanel(QWidget *parent)
    : QWidget(parent)
{

    buildUi();
    buildLayout();
    connectSignals();
}
    

void BeamPanel::buildUi(){
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 4);

    m_histogramPlot    = new QTextEdit(this);
    m_randomButton = new QPushButton(tr("Random"), this);

    rootLayout->addWidget(m_histogramPlot, 1);
    rootLayout->addWidget(m_randomButton, 0);
}

void BeamPanel::buildLayout(){

}

void BeamPanel::connectSignals(){

}