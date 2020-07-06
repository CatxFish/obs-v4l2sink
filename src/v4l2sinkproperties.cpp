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

#include "v4l2sinkproperties.h"
#include "ui_v4l2sinkproperties.h"
#include <obs-frontend-api.h>
#include <util/config-file.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <QDir>

V4l2sinkProperties::V4l2sinkProperties(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::V4l2sinkProperties)
{
	ui->setupUi(this);
	connect(ui->pushButton_start,SIGNAL(clicked(bool)), this, SLOT(onStart()));
	connect(ui->pushButton_stop,SIGNAL(clicked(bool)), this, SLOT(onStop()));

	config_t* config = obs_frontend_get_global_config();
	config_set_default_bool(config, "V4l2sink", "AutoStart", false);
	config_set_default_string(config, "V4l2sink", "DevicePath", "/dev/video0");
	config_set_default_string(config, "V4l2sink", "Format", V4L2SINK_YUV420);

	bool autostart = config_get_bool(config, "V4l2sink", "AutoStart");
	const char* device = config_get_string(config, "V4l2sink", "DevicePath");
	const char* format = config_get_string(config, "V4l2sink", "Format");

	init_devices(device);

	ui->checkBox_auto->setChecked(autostart);

	QMapIterator<QString, QString> i(devices);
	while (i.hasNext()) {
		i.next();
		ui->comboBox_device->addItem(i.value(), i.key());
	}
	ui->comboBox_device->setCurrentIndex(ui->comboBox_device->findData(device));

	ui->comboBox_format->addItem(V4L2SINK_YUV420, V4L2SINK_YUV420);
	ui->comboBox_format->addItem(V4L2SINK_NV12, V4L2SINK_NV12);
	ui->comboBox_format->addItem(V4L2SINK_YUY2, V4L2SINK_YUY2);
	ui->comboBox_format->addItem(V4L2SINK_RGB32, V4L2SINK_RGB32);
	ui->comboBox_format->setCurrentIndex(
		ui->comboBox_format->findText(format));

	ui->label_warning->setStyleSheet("QLabel { color : red; }");
	enableStart(true);

	if(autostart)
		onStart();

}

V4l2sinkProperties::~V4l2sinkProperties()
{
	saveSettings();
	v4l2sink_release();
	delete ui;
}

void V4l2sinkProperties::init_devices(const char* device)
{
	QDir dir("/sys/class/video4linux");
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::Name);
	QFileInfoList list = dir.entryInfoList();

	for (int i = 0; i < list.size(); ++i) {
		int fd;
		uint32_t caps;
		struct v4l2_capability video_cap;

		QFileInfo fileInfo = list.at(i);
		QString fileName = fileInfo.fileName();

		if (!fileName.startsWith("video"))
			continue;
		QString devPath = QString("/dev/%1").arg(fileName);

		if ((fd = ::open(devPath.toLatin1(), O_RDWR | O_NONBLOCK)) == -1) {
			continue;
		}

		if (::ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1) {
			::close(fd);
			continue;
		}

		if (video_cap.capabilities & V4L2_CAP_DEVICE_CAPS) {
			caps = video_cap.device_caps;
		} else {
			caps = video_cap.capabilities;
		}

		if (!(caps & V4L2_CAP_VIDEO_CAPTURE)) {
			::close(fd);
			continue;
		}

		if (strcmp((char*) video_cap.driver, "v4l2 loopback") != 0) {
			::close(fd);
			continue;
		}

		/* make sure device names are unique */
		char dev_name[68];
		sprintf(dev_name, "%s (%s)", video_cap.card, video_cap.bus_info);
		devices.insert(devPath, QString::fromLocal8Bit(dev_name));
		::close(fd);
	}

	QString cur_device = QString::fromLocal8Bit(device);
	if (!devices.contains(cur_device)) {
		devices.remove(cur_device);
	}
}

void V4l2sinkProperties::closeEvent(QCloseEvent *event)
{
	saveSettings();	
}

void V4l2sinkProperties::saveSettings()
{
	bool autostart = ui->checkBox_auto->isChecked();
	QByteArray ba_dev_name = ui->comboBox_device->currentData().toString().toUtf8();
	QByteArray ba_format = ui->comboBox_format->currentData().toString().
		toUtf8();
	config_t* config = obs_frontend_get_global_config();
	if(config){
		config_set_bool(config, "V4l2sink", "AutoStart", autostart);
		config_set_string(config, "V4l2sink", "DevicePath", 
			ba_dev_name.constData());
		config_set_string(config, "V4l2sink", "Format", 
			ba_format.constData());
	}

}
void V4l2sinkProperties::onStart()
{
	QByteArray ba_format = ui->comboBox_format->currentData().toString()
		.toUtf8();
	QByteArray ba_dev_name = ui->comboBox_device->currentData().toString().toUtf8();
	signal_handler_t *handler = v4l2sink_get_signal_handler();
	signal_handler_connect(handler, "v4l2close", output_stopped , this);
	enableStart(false);
	setWarningText("");
	saveSettings();
	v4l2sink_enable(ba_dev_name.constData(), ba_format.constData());
}

void V4l2sinkProperties::onStop()
{
	v4l2sink_disable();
}

void V4l2sinkProperties::enableStart(bool enable)
{
	ui->pushButton_start->setEnabled(enable);
	ui->pushButton_stop->setEnabled(!enable);
}

void V4l2sinkProperties::setWarningText(const char *msg)
{
	ui->label_warning->setText(msg);
}

static void output_stopped(void *data, calldata_t *cd)
{
	auto page = (V4l2sinkProperties*) data;
	auto output = (obs_output_t*) calldata_ptr(cd, "output");
	bool opening = calldata_bool(cd, "opening");
	const char* msg = calldata_string(cd, "msg");

	if (opening)
		page->setWarningText(msg);
		
	signal_handler_t *handler = obs_output_get_signal_handler(output);
	page->enableStart(true);
	signal_handler_disconnect(handler, "v4l2close", output_stopped , page);
}





