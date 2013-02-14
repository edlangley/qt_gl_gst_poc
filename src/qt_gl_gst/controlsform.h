#ifndef CONTROLSFORM_H
#define CONTROLSFORM_H

#include <QWidget>

class GLWidget;

namespace Ui {
class ControlsForm;
}

class ControlsForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit ControlsForm(GLWidget *glWidgetToInsert, QWidget *parent = 0);
    ~ControlsForm();

protected:
    void closeEvent(QCloseEvent* event);
    
private:
    Ui::ControlsForm *ui;
};

#endif // CONTROLSFORM_H
