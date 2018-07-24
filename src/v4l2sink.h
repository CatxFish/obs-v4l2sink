#ifndef V4L2SINK_H
#define V4L2SINK_H
#include <callback/signal.h>
void v4l2sink_enable(const char *dev_name, const char *format);
void v4l2sink_disable();
void v4l2sink_release();
signal_handler_t* v4l2sink_get_signal_handler();
#endif // V4L2SINK_H
