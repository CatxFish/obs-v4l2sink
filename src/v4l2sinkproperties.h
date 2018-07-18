#ifndef V4L2SINKPROPERTIES_H
#define V4L2SINKPROPERTIES_H

#include <QDialog>

namespace Ui {
class V4l2sinkProperties;
}

class V4l2sinkProperties : public QDialog
{
    Q_OBJECT

public:
    explicit V4l2sinkProperties(QWidget *parent = 0);
    ~V4l2sinkProperties();

private Q_SLOTS:
    void onStart();
    void onStop();

private:
    Ui::V4l2sinkProperties *ui;
};

#endif // V4L2SINKPROPERTIES_H
