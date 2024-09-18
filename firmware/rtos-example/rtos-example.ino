#define LED_PIN 2

TaskHandle_t taskBlink1Handle, taskBlink2Handle;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  xTaskCreate(
    
    blink1,            // Function name of the task
    "Blink 1",         // Name of the task (e.g. for debugging)
    2048,              // Stack size (bytes)
    NULL,              // Parameter to pass
    1,                 // Task priority
    &taskBlink1Handle  // Task handle
  );
  xTaskCreate(
    blink2,            // Function name of the task
    "Blink 2",         // Name of the task (e.g. for debugging)
    2048,              // Stack size (bytes)
    NULL,              // Parameter to pass
    1,                 // Task priority
    &taskBlink2Handle  // Task handle
  );
}
void blink1(void *parameter) {
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
  }
}
void blink2(void *parameter) {
  int count = 0;
  char buffer[50];
  constexpr int n = sizeof(buffer) / sizeof(char);

  while (1) {
    snprintf(buffer, n, "task count: %d\n", count);
    buffer[n - 1] = '\0';
    Serial.print((const char *)buffer);

    snprintf(buffer, n, "task high watermark: %d\n",
             uxTaskGetStackHighWaterMark(taskBlink2Handle));
    Serial.print((const char *)buffer);

    count++;
    delay(333);
  }
}
void loop() {
}