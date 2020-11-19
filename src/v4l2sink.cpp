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


#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QMainWindow>
#include <QAction>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "v4l2sink.h"
#include "v4l2sinkproperties.h"

#define V4L2SINK_SUCCESS_OPEN  0
#define V4L2SINK_ERROR_OPEN    1
#define V4L2SINK_ERROR_FORMAT  2
#define V4L2SINK_ERROR_OTHER   3

struct v4l2sink_data{
	obs_output_t *output = nullptr;
	bool active = false;
	int v4l2_fd = 0;
	int width = 0;
	int height = 0;
	int frame_size = 0;
	uint32_t format = V4L2_PIX_FMT_YUYV;
};

static inline enum video_format v4l2_to_obs_video_format(uint_fast32_t format)
{
	switch (format) {
	case V4L2_PIX_FMT_YVYU:   return VIDEO_FORMAT_YVYU;
	case V4L2_PIX_FMT_YUYV:   return VIDEO_FORMAT_YUY2;
	case V4L2_PIX_FMT_UYVY:   return VIDEO_FORMAT_UYVY;
	case V4L2_PIX_FMT_NV12:   return VIDEO_FORMAT_NV12;
	case V4L2_PIX_FMT_YUV420: return VIDEO_FORMAT_I420;
	case V4L2_PIX_FMT_YVU420: return VIDEO_FORMAT_I420;
#ifdef V4L2_PIX_FMT_XBGR32
	case V4L2_PIX_FMT_XBGR32: return VIDEO_FORMAT_BGRX;
#endif
	case V4L2_PIX_FMT_BGR32:  return VIDEO_FORMAT_BGRA;
#ifdef V4L2_PIX_FMT_ABGR32
	case V4L2_PIX_FMT_ABGR32: return VIDEO_FORMAT_BGRA;
#endif
	default:                  return VIDEO_FORMAT_NONE;
	}
}

static inline uint32_t string_to_v4l2_format(const char* format)
{
	if(strcmp(format, V4L2SINK_NV12)==0)
		return V4L2_PIX_FMT_NV12;
	else if(strcmp(format, V4L2SINK_YUV420)==0)
		return V4L2_PIX_FMT_YUV420;
	else if (strcmp(format, V4L2SINK_RGB32)==0)
		return V4L2_PIX_FMT_BGR32;
	else
		return V4L2_PIX_FMT_YUYV;
}

V4l2sinkProperties* prop;
obs_output_t* v4l2_out;

void v4l2sink_signal_init(const char *signal)
{
	signal_handler_t *handler = v4l2sink_get_signal_handler();
	signal_handler_add(handler,signal);
}

void v4l2sink_signal_stop(const char *msg, bool opening)
{
	struct calldata call_data;
	calldata_init(&call_data);
	calldata_set_string(&call_data, "msg", msg);
	calldata_set_bool(&call_data,"opening",opening);
	signal_handler_t *handler = v4l2sink_get_signal_handler();
	signal_handler_signal(handler, "v4l2close", &call_data);
	calldata_free(&call_data);
}

bool v4l2device_set_format(void *data,struct v4l2_format *format)
{
	v4l2sink_data *out_data = (v4l2sink_data*)data;
	format->fmt.pix.width = out_data->width;
	format->fmt.pix.height = out_data->height;
	format->fmt.pix.pixelformat = out_data->format;
	format->fmt.pix.sizeimage = out_data->frame_size;
	return true;
}

int v4l2device_framesize(void *data)
{	
	v4l2sink_data *out_data = (v4l2sink_data*)data;
	switch(out_data->format){
	
	case V4L2_PIX_FMT_YVYU:   
	case V4L2_PIX_FMT_YUYV:   
	case V4L2_PIX_FMT_UYVY: 
		return out_data->width * out_data->height * 2;
	case V4L2_PIX_FMT_YUV420: 
	case V4L2_PIX_FMT_YVU420:
		return out_data->width * out_data->height * 3 / 2;
#ifdef V4L2_PIX_FMT_XBGR32
	case V4L2_PIX_FMT_XBGR32:
#endif
#ifdef V4L2_PIX_FMT_ABGR32
	case V4L2_PIX_FMT_ABGR32: 
#endif
	case V4L2_PIX_FMT_BGR32:
		return out_data->width * out_data->height * 4;				
	}
	return 0;
}

int v4l2device_open(void *data)
{
	v4l2sink_data *out_data = (v4l2sink_data*)data;
	struct v4l2_format v4l2_fmt;
	int width,height,ret = 0;
	struct v4l2_capability capability;
	enum video_format format;
	video_t *video = obs_output_video(out_data->output);

	obs_data_t *settings = obs_output_get_settings(out_data->output);
	out_data->v4l2_fd = open(obs_data_get_string(settings, "device_name")
		, O_RDWR);
	out_data->format = string_to_v4l2_format(
		obs_data_get_string(settings, "format"));
	out_data->frame_size = v4l2device_framesize(data);	
	obs_data_release(settings); 

	if(out_data->v4l2_fd  < 0){
		printf("v4l2 device open fail\n");
		return V4L2SINK_ERROR_OPEN;
	}

	if (ioctl(out_data->v4l2_fd, VIDIOC_QUERYCAP, &capability) < 0){ 
		printf("v4l2 device qureycap fail\n");		
		return V4L2SINK_ERROR_FORMAT;
	}

	v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret = ioctl(out_data->v4l2_fd, VIDIOC_G_FMT, &v4l2_fmt);

	if(ret<0){		
		printf("v4l2 device getformat fail\n");
		return V4L2SINK_ERROR_FORMAT;
	}

	v4l2device_set_format(data,&v4l2_fmt);
	ret = ioctl(out_data->v4l2_fd, VIDIOC_S_FMT, &v4l2_fmt);

	if(ret<0){		
		printf("v4l2 device setformat fail\n");
		return V4L2SINK_ERROR_FORMAT;
	}

	ret = ioctl(out_data->v4l2_fd, VIDIOC_G_FMT, &v4l2_fmt);

	if(ret<0){		
		printf("v4l2 device getformat fail\n");
		return V4L2SINK_ERROR_FORMAT;
	}

	if(out_data->format != v4l2_fmt.fmt.pix.pixelformat){
		printf("v4l2 format not support\n");
		return V4L2SINK_ERROR_FORMAT;
	}


	width = (int32_t)obs_output_get_width(out_data->output);
	height = (int32_t)obs_output_get_height(out_data->output);
	format = v4l2_to_obs_video_format(v4l2_fmt.fmt.pix.pixelformat);

	if(format == VIDEO_FORMAT_NONE){
		printf("v4l2 conversion format not support\n");
		return V4L2SINK_ERROR_FORMAT;
	}
	
	if(width!= v4l2_fmt.fmt.pix.width ||
	height!= v4l2_fmt.fmt.pix.height ||
	format!= video_output_get_format(video)){
		struct video_scale_info conv;
		conv.format = format;
		conv.width = v4l2_fmt.fmt.pix.width;	
		conv.height = v4l2_fmt.fmt.pix.height;
		out_data->frame_size = v4l2_fmt.fmt.pix.sizeimage;
		obs_output_set_video_conversion(out_data->output,&conv);
	}
	else
		obs_output_set_video_conversion(out_data->output,NULL);

	return V4L2SINK_SUCCESS_OPEN;
}



static void v4l2device_close(void *data)
{
	v4l2sink_data *out_data = (v4l2sink_data*)data;
	close(out_data->v4l2_fd);
}

static const char *v4l2sink_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("V4l2sink");
}

static void v4l2sink_destroy(void *data)
{
	v4l2sink_data *out_data = (v4l2sink_data*)data;
	if (out_data){
		bfree(out_data);
	}
}
static void *v4l2sink_create(obs_data_t *settings, obs_output_t *output)
{
	v4l2sink_data *data = (v4l2sink_data *)bzalloc(sizeof(
		struct v4l2sink_data));
	data->output = output;
	UNUSED_PARAMETER(settings);
	return data;
}

static bool v4l2sink_start(void *data)
{
	v4l2sink_data *out_data = (v4l2sink_data*)data;
	out_data->width = (int32_t)obs_output_get_width(out_data->output);
	out_data->height = (int32_t)obs_output_get_height(out_data->output);
	int ret = v4l2device_open(data);

	if(ret!= V4L2SINK_SUCCESS_OPEN){
		switch (ret) {
		case V4L2SINK_ERROR_OPEN: 
			v4l2sink_signal_stop("device open failed", true);
			break;
		case V4L2SINK_ERROR_FORMAT:
			v4l2sink_signal_stop("format not support", true);
			break;
		default:
			v4l2sink_signal_stop("device open failed", true);
		}
		return false;
	}
	
	if(!obs_output_can_begin_data_capture(out_data->output,0)){
		v4l2sink_signal_stop("start failed", true);
		return false;
	}

	out_data->active = true;
	return obs_output_begin_data_capture(out_data->output, 0);	
}

static void v4l2sink_stop(void *data, uint64_t ts)
{
	v4l2sink_data *out_data = (v4l2sink_data*)data;

	if(out_data->active){
		out_data->active = false;
		obs_output_end_data_capture(out_data->output);	
		v4l2device_close(data);
		v4l2sink_signal_stop("stop", false);
	}
	
}

obs_properties_t* v4l2sink_getproperties(void *data)
{
	UNUSED_PARAMETER(data);

	obs_properties_t* props = obs_properties_create();
	obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);

	obs_properties_add_text(props, "v4l2sink_name",
		obs_module_text("V4l2sink.name"), OBS_TEXT_DEFAULT);
	return props;
}

static void v4l2sink_videotick(void *param, struct video_data *frame)
{
	v4l2sink_data *out_data = (v4l2sink_data*)param;
	if(out_data->active){
		size_t bytes = write(out_data->v4l2_fd, frame->data[0], 
			out_data->frame_size);
	}
}

struct obs_output_info create_output_info()
{
	struct obs_output_info output_info = {};
	output_info.id = "v4l2sink";
	output_info.flags = OBS_OUTPUT_VIDEO;
	output_info.get_name = v4l2sink_getname;
	output_info.create = v4l2sink_create;
	output_info.destroy = v4l2sink_destroy;
	output_info.start = v4l2sink_start;
	output_info.stop = v4l2sink_stop;
	output_info.raw_video = v4l2sink_videotick;
	output_info.get_properties = v4l2sink_getproperties;
	return output_info;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("v4l2sink", "en-US")

bool obs_module_load(void)
{
	obs_output_info v4l2sink_info = create_output_info();
	obs_register_output(&v4l2sink_info);
	obs_data_t *settings = obs_data_create();
	v4l2_out = obs_output_create("v4l2sink", "V4l2sink",settings, NULL);
	obs_data_release(settings);
	v4l2sink_signal_init("void v4l2close(string msg, bool opening)");

	QMainWindow* main_window = (QMainWindow*)obs_frontend_get_main_window();
	QAction *action = (QAction*)obs_frontend_add_tools_menu_qaction(
		obs_module_text("V4l2sink"));

	obs_frontend_push_ui_translation(obs_module_get_string);
	prop = new V4l2sinkProperties(main_window);
	obs_frontend_pop_ui_translation();

	auto menu_cb = []
	{
		prop->setVisible(!prop->isVisible());
	};

	action->connect(action, &QAction::triggered, menu_cb);

    	return true;
}

void obs_module_unload()
{
}

void v4l2sink_release()
{
	obs_output_stop(v4l2_out);
	obs_output_release(v4l2_out);
}

void v4l2sink_enable(const char *dev_name, const char *format)
{
	obs_data_t *settings = obs_output_get_settings(v4l2_out);
	obs_data_set_string(settings, "device_name", dev_name);
	obs_data_set_string(settings, "format", format);
	obs_output_update(v4l2_out,settings);
	obs_data_release(settings);
	obs_output_start(v4l2_out);	
}

void v4l2sink_disable()
{
	obs_output_stop(v4l2_out);
}

signal_handler_t* v4l2sink_get_signal_handler()
{
	return obs_output_get_signal_handler(v4l2_out);	
}
