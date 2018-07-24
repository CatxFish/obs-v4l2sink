#ifndef V4L2SINKPROPERTIES_H
#define V4L2SINKPROPERTIES_H

#include <QDialog>
#include "v4l2sink.h"

#define V4L2SINK_NV12   "NV12"
#define V4L2SINK_YUV420 "YUV420"
#define V4L2SINK_YUY2   "YUY2"
#define V4L2SINK_RGB32  "RGB32"


namespace Ui {
class V4l2sinkProperties;
}

class V4l2sinkProperties : public QDialog
{
	Q_OBJECT

public:
	explicit V4l2sinkProperties(QWidget *parent = 0);
	~V4l2sinkProperties();
	void enableStart(bool enable);

private Q_SLOTS:
	void onStart();
	void onStop();

private:
	Ui::V4l2sinkProperties *ui;
};

static void output_started(void *data, calldata_t *cd);
static void output_stopped(void *data, calldata_t *cd);


#endif // V4L2SINKPROPERTIES_H
