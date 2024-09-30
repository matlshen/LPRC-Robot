#!/bin/bash

# TCP to Serial
socat PTY,link=/dev/ttyV0,raw,echo=0,b115200 TCP:localhost:9999 &

# Serial to TCP
socat TCP-LISTEN:9999,reuseaddr,reuseport,fork PTY,link=/dev/ttyV0,raw,echo=0,b115200 &

# Start microros agent
ros2 run micro_ros_agent micro_ros_agent serial -D /dev/ttyV0