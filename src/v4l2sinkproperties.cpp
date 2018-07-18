#include "v4l2sinkproperties.h"
#include "ui_v4l2sinkproperties.h"
#include <obs-frontend-api.h>
#include <util/config-file.h>

V4l2sinkProperties::V4l2sinkProperties(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::V4l2sinkProperties)
{
    ui->setupUi(this);
    connect(ui->pushButton_start,SIGNAL(clicked(bool)),this,SLOT(onStart()));
    connect(ui->pushButton_stop,SIGNAL(clicked(bool)),this,SLOT(onStop()));

    config_t* config = obs_frontend_get_global_config();
    config_set_default_bool(config, "V4l2sink", "AutoStart", false);
    config_set_default_string(config, "V4l2sink", "DevicePath","/dev/video1");
    bool autostart = config_get_bool(config, "V4l2sink", "AutoStart");
    const char* device = config_get_string(config, "V4l2sink", "DevicePath");

    ui->checkBox_auto->setChecked(autostart);
    ui->lineEdit_dev->setText(device);

}

V4l2sinkProperties::~V4l2sinkProperties()
{
    delete ui;
}

void V4l2sinkProperties::onStart()
{

}

void V4l2sinkProperties::onStop()
{

}
