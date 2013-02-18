#include "controlsform.h"
#include "ui_controlsform.h"

#include "glwidget.h"

ControlsForm::ControlsForm(GLWidget *glWidgetToInsert, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlsForm)
{
    ui->setupUi(this);

    ui->glWidgetLayout->addWidget(glWidgetToInsert);

    QObject::connect(ui->cycBackGndPushButton, SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(cycleBackgroundSlot()));
    QObject::connect(ui->cycModShadPushButton, SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(cycleModelShaderSlot()));
    QObject::connect(ui->cycVidShadPushButton, SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(cycleVidShaderSlot()));
    QObject::connect(ui->exitPushButton,       SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(exitSlot()));
    QObject::connect(ui->loadAlphaPushButton,  SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(loadAlphaSlot()));
    QObject::connect(ui->loadModelPushButton,  SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(loadModelSlot()));
    QObject::connect(ui->loadVidPushButton,    SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(loadVideoSlot()));
    QObject::connect(ui->resetPosPushButton,   SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(resetPosSlot()));
    QObject::connect(ui->showYUVPushButton,    SIGNAL(clicked(bool)),     glWidgetToInsert, SLOT(showYUVWindowSlot()));
    QObject::connect(ui->rotateCheckBox,       SIGNAL(toggled(bool)),     glWidgetToInsert, SLOT(rotateToggleSlot(bool)));
    QObject::connect(ui->stackVidsCheckBox,    SIGNAL(stateChanged(int)), glWidgetToInsert, SLOT(stackVidsToggleSlot(int)));

    QObject::connect(glWidgetToInsert, SIGNAL(rotateStateChanged(bool)),    ui->rotateCheckBox,    SLOT(setChecked(bool)));
    QObject::connect(glWidgetToInsert, SIGNAL(stackVidsStateChanged(bool)), ui->stackVidsCheckBox, SLOT(setChecked(bool)));

}

ControlsForm::~ControlsForm()
{
    delete ui;
}

void ControlsForm::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);

    // At the mo, tell parent to close too.
    QWidget* _parent = dynamic_cast<QWidget*>(parent());
    if(_parent)
        _parent->close();
}
