#include "threads.h"
#include "board_config.h"
#include "microros.h"
#include "motor.h"

Motor motorFL(FIN1B, FIN2B);
Motor motorFR(FIN1A, FIN2A);
Motor motorBL(BIN1A, BIN2A);
Motor motorBR(BIN1B, BIN2B);

void twist_callback(const void *msgin) {
    const geometry_msgs__msg__Twist * msgt = (const geometry_msgs__msg__Twist *)msgin;
    // if velocity in x direction is 0 turn off LED, if 1 turn on LED
    digitalWrite(LED_PIN, (msgt->linear.x == 0) ? HIGH : LOW);
    // msg_data.data = msgt->linear.x;
    // RCSOFTCHECK(rcl_publish(&publisher, &msg_data, NULL));

    int scaled_vx = msgt->linear.x * 100;
    int scaled_vy = msgt->linear.y * 100;
    int scaled_twist = msgt->angular.z * 50;

    int fl_speed = scaled_vx - scaled_vy - scaled_twist;
    int fr_speed = scaled_vx + scaled_vy + scaled_twist;
    int bl_speed = scaled_vx + scaled_vy - scaled_twist;
    int br_speed = scaled_vx - scaled_vy + scaled_twist;

    motorFL.SetSpeed(fl_speed);
    motorFR.SetSpeed(fr_speed);
    motorBL.SetSpeed(bl_speed);
    motorBR.SetSpeed(br_speed);
}