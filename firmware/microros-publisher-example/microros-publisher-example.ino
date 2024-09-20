#include <stdio.h>

#include "board_config.h"
#include "threads.h"
#include "microros.h"
#include "motor.h"

void setup() {
    Motor motorFL(FIN1B, FIN2B);
    Motor motorFR(FIN1A, FIN2A);
    Motor motorBL(BIN1A, BIN2A);
    Motor motorBR(BIN1B, BIN2B);


    // motorFL.SetSpeed(100);
    // motorFR.SetSpeed(100);
    // motorBL.SetSpeed(100);
    // motorBR.SetSpeed(100);
    // delay(1000);
    // motorFL.SetSpeed(-100);
    // motorFR.SetSpeed(-100);
    // motorBL.SetSpeed(-100);
    // motorBR.SetSpeed(-100);
    // delay(1000);
    // motorFL.Stop();
    // motorFR.Stop();
    // motorBL.Stop();
    // motorBR.Stop();
    // delay(1000);

    microros_setup();
}

void loop() {
    delay(100);
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}