#include "microros.h"
#include "board_config.h"
#include "threads.h"

// Publish an integer topic
rcl_publisher_t publisher;
std_msgs__msg__Int32 msg_data;

// Subscribe to twist topic
rcl_subscription_t subscriber;
geometry_msgs__msg__Twist msg_twist;

rclc_executor_t executor;
rclc_executor_t sub_executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

rcl_timer_t timer;

void error_loop(){
    while(1){
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(100);
    }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
    RCLC_UNUSED(last_call_time);
    if (timer != NULL) {
        RCSOFTCHECK(rcl_publish(&publisher, &msg_data, NULL));
        msg_data.data++;
    }
}

void microros_setup() {
    set_microros_transports();
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  
    delay(2000);

    allocator = rcl_get_default_allocator();

    //create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    // create node
    RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

    // create timer,
    const unsigned int timer_timeout = 1000;
    RCCHECK(rclc_timer_init_default(
        &timer,
        &support,
        RCL_MS_TO_NS(timer_timeout),
        timer_callback));

    // create publisher
    RCCHECK(rclc_publisher_init_default(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "micro_ros_arduino_node_publisher"));

    // Create subscriber
    RCCHECK(rclc_subscription_init_default(
        &subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
        "cmd_vel"));

    // create executors
    RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg_twist, &twist_callback, ON_NEW_DATA));

    msg_data.data = 0;
}