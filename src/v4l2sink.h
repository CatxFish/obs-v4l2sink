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

#ifndef V4L2SINK_H
#define V4L2SINK_H
#include <callback/signal.h>
void v4l2sink_enable(const char *dev_name, const char *format);
void v4l2sink_disable();
void v4l2sink_release();
signal_handler_t* v4l2sink_get_signal_handler();
#endif // V4L2SINK_H
