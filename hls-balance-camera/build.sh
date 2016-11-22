fdevent_src="/root/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src"
fdevent_inc="/root/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src/"
arts_common_src="/root/CNSROOT/build/apps/arts/common/lib/debug"
interface_src="/root/CNSROOT/build/apps/arts/interfaces/lib/debug"
protobuf="/root/CNSROOT/build/protobuf/lib"
json="/arts/lib/"

g++ -g -o check_camera check_camera.cpp -I $fdevent_inc -L$fdevent_src -L$arts_common_src -L$interface_src -L$protobuf -L$json -lfdevent -lsctp -larts_interface -larts_common -L$protobuf -lprotobuf -ljson_linux-gcc-4.4.7_libmt



#g++ -g -o check_camera check_camera.cpp  -I /home/gst/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src/  -L/home/gst/CNSROOT/external/lighttpd/lighttpd-1.5.0-r2746/src -L/home/gst/CNSROOT/build/apps/arts/common/lib/debug -L/home/gst/CNSROOT/build/apps/arts/interfaces/lib/debug  -lfdevent -lsctp -larts_interface -larts_common  -L/home/gst/CNSROOT/build/protobuf/lib -lprotobuf
