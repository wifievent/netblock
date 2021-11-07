#ifndef POLICYTIMEEDIT_H
#define POLICYTIMEEDIT_H

#include <QTimeEdit>

class PolicyTimeEdit : public QTimeEdit
{
    Q_OBJECT
public:
    explicit PolicyTimeEdit(QWidget *parent = nullptr);
    void stepBy(int steps) override;
};

#endif // POLICYTIMEEDIT_H
