# Gowin Build Script for Gesture Recognition
set_device GW1N-UV1P5QN48XFC7/I6 -device_version C
add_file src/gesture_top.v
add_file src/spi_slave.v
add_file src/imu_buffer.v
add_file src/gesture_bcnn.v
add_file src/gesture.cst

set_option -top_module gesture_top
set_option -output_base_name gesture_recognition

run all
