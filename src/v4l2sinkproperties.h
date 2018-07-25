/******************************************************************************
    obs-v4l2sink
    Copyright (C) 2018 by CatxFish
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

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
	void setWarningText(const char *msg);
	void closeEvent(QCloseEvent *event);
	void saveSettings();

private Q_SLOTS:
	void onStart();
	void onStop();

private:
	Ui::V4l2sinkProperties *ui;
};

static void output_started(void *data, calldata_t *cd);
static void output_stopped(void *data, calldata_t *cd);


#endif // V4L2SINKPROPERTIES_H
