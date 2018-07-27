# obs-v4l2sink

This plugin provides output ability to v4l2 device . It is basically a Linux version of [obs-virtual-cam](https://github.com/CatxFish/obs-virtual-cam), but only contains the video sink part. You can use it with [v4l2loopback](https://github.com/umlaeute/v4l2loopback) to achieve cross-program image transfer between obs-studio and third party software supporting Video4Linux.  

# Usage with v4l2loopback
- Make sure [load v4l2loopback module](https://github.com/umlaeute/v4l2loopback#run) and check the device path.
- Open OBS and click tools > v4l2sink .
- Fill device path and choose video format.
- Click start.

# Build
```
git clone https://github.com/CatxFish/obs-v4l2sink.git
cd obs-v4l2sink
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR="<libobs path>" -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
sudo make install
``` 
