#include <obs-module.h>
#include <obs-frontend-api.h>
#include <QMainWindow>
#include <QAction>
#include "v4l2sink.h"
#include "v4l2sinkproperties.h"

struct v4l2sink_data{
        obs_output_t *output = nullptr;
};

V4l2sinkProperties* prop;

static const char *v4l2sink_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("V4l2sink");
}

static void v4l2sink_destroy(void *data)
{
    v4l2sink_data *out_data = (v4l2sink_data*)data;
    if (out_data)
        bfree(data);
}
static void *v4l2sink_create(obs_data_t *settings, obs_output_t *output)
{
    v4l2sink_data *data =
        (v4l2sink_data *)bzalloc(sizeof(struct v4l2sink_data));

    data->output = output;
    UNUSED_PARAMETER(settings);
    return data;
}

static void v4l2sink_update(void *data, obs_data_t *settings)
{


}

static bool v4l2sink_start(void *data)
{

}

static void v4l2sink_stop(void *data, uint64_t ts)
{

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
        output_info.update = v4l2sink_update;
        output_info.get_properties = v4l2sink_getproperties;

        return output_info;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("v4l2sink", "en-US")

bool obs_module_load(void)
{
    obs_output_info v4l2sink_info = create_output_info();
    obs_register_output(&v4l2sink_info);

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

void v4l2sink_enable(void)
{

}
void v4l2sink_disable(void)
{


}
