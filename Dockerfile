FROM ros:humble-ros-core

# Install necessary packages
RUN apt-get update && apt-get install -y \
    ros-humble-desktop \
    build-essential \
    cmake \
    git \
    ros-humble-ament-cmake \
    python3-ament-package \
    python3-colcon-common-extensions \
    python3-rosdep2 \
    socat \
    netcat \
    picocom \
    telnet

RUN rm -rf /var/lib/apt/lists/*

# Add ROS setup to bashrc
RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc

# Create the micro-ROS workspace and clone the micro-ROS setup repository
RUN mkdir -p ~/microros_ws/src && \
    cd ~/microros_ws && \
    git clone -b $ROS_DISTRO https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup

# Update rosdep, install dependencies, and build the workspace
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    cd ~/microros_ws && \
    rosdep update && \
    apt update && \
    rosdep install --from-paths src --ignore-src -y && \
    colcon build && \
    source install/local_setup.bash && \
    ros2 run micro_ros_setup create_agent_ws.sh && \
    ros2 run micro_ros_setup build_agent.sh"

# Add microROS setup to bashrc
RUN echo "source ~/microros_ws/install/local_setup.bash" >> /root/.bashrc

# Add microros startup configuration script
COPY ros2_ws/ /root/workspace/
RUN chmod +x /root/workspace/microros_setup.sh

# Expose ports
EXPOSE 9999

ENTRYPOINT ["/bin/bash"]

#ENV DISPLAY=host.docker.internal:0