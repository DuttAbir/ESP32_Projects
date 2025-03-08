static const uint32_t timer_clock = 1000000;
static const uint64_t timer_max_count = 1000000;

static const int led_pin = 2;

static hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer(){
  int pin_state = digitalRead(led_pin);
  digitalWrite(led_pin, !pin_state);
}

void setup() {
  pinMode(led_pin, OUTPUT);

  timer = timerBegin(timer_clock);

  timerAttachInterrupt(timer, &onTimer);

  timerAlarm(timer, timer_max_count, true, 0);
}

void loop() {
}
