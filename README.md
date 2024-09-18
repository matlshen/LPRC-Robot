# LPRC-Robot
### Running MicroROS Example
1. Compile `ser2net` for host and start a bridge
```bash
gcc ./src/ser2net -o ser2net
ser2net -s <ESP32 serial port> -b 115200 -p 9999
```
2. Build and run Docker container
```
docker build -t ros2-humble-microros
docker run -it --rm -p 9999:9999 --name ros2-humble-microros-container ros2-humble-microros 
```
3. From within the Docker container, run the startup script
```
~/workspace/microros_setup.sh
```
4. Hit the reset button on the ESP32
5. Check that connection in Docker container is successful