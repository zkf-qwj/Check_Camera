fdevent_src="/home/gst/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src"
fdevent_inc="/home/gst/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src/"
arts_common_src="/home/gst/CNSROOT/build/apps/arts/common/lib/debug"
interface_src="/home/gst/CNSROOT/build/apps/arts/interfaces/lib/debug"
protobuf="/home/gst/CNSROOT/build/protobuf/lib"

g++ -g -o check_camera check_camera.cpp -I $fdevent_inc -L$fdevent_src -L$arts_common_src -L$interface_src -L$protobuf -lfdevent -lsctp -larts_interface -larts_common -L$protobuf -lprotobuf



#g++ -g -o check_camera check_camera.cpp  -I /home/gst/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src/  -L/home/gst/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src -L/home/gst/CNSROOT/build/apps/arts/common/lib/debug -L/home/gst/CNSROOT/build/apps/arts/interfaces/lib/debug  -lfdevent -lsctp -larts_interface -larts_common  -L/home/gst/CNSROOT/build/protobuf/lib -lprotobuf
