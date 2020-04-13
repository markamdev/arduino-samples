// output constants
const int LED_CAR_R = 2;
const int LED_CAR_Y = 3;
const int LED_CAR_G = 4;

const int LED_MAN_R = 7;
const int LED_MAN_G = 6;

const int BUTTON_MAN = 10;

// utility constants
const int DELAY_ALL_STOP = 500;
const int DELAY_MAN_WALK = 1000;
const int DELAY_CAR_READY = 500;
const int DELAY_CAR_GO = 3000;
const int DELAY_CAR_WARNING = 500;

void setup() {
  pinMode(LED_CAR_R, OUTPUT);
  pinMode(LED_CAR_Y, OUTPUT);
  pinMode(LED_CAR_G, OUTPUT);
  pinMode(LED_MAN_R, OUTPUT);
  pinMode(LED_MAN_G, OUTPUT);

  pinMode(BUTTON_MAN, INPUT);
}

enum state_name {
  ALL_STOP = 0,
  MAN_WALK,
  CAR_READY,
  CAR_GO,
  CAR_WARNING
};

const int states[5][5] = {
  // CAR_G | CAR_Y | CAR_R | MAN_G | MAN_R
  { LOW, LOW, HIGH, LOW, HIGH }, // all stop
  { LOW, LOW, HIGH, HIGH, LOW }, // man walk
  { LOW, HIGH, HIGH, LOW, HIGH }, // car ready
  { HIGH, LOW, LOW, LOW, HIGH }, // car go
  { LOW, HIGH, LOW, LOW, HIGH } // car warning
};

void set_state(int st)
{
  digitalWrite(LED_CAR_G, states[st][0]);
  digitalWrite(LED_CAR_Y, states[st][1]);
  digitalWrite(LED_CAR_R, states[st][2]);
  digitalWrite(LED_MAN_G, states[st][3]);
  digitalWrite(LED_MAN_R, states[st][4]);
}

void loop() {
  set_state(ALL_STOP);
  delay(DELAY_ALL_STOP);
  set_state(MAN_WALK);
  delay(DELAY_MAN_WALK);
  set_state(ALL_STOP);
  delay(DELAY_ALL_STOP);
  set_state(CAR_READY);
  delay(DELAY_CAR_READY);
  set_state(CAR_GO);
  delay(DELAY_CAR_GO);
  set_state(CAR_WARNING);
  delay(DELAY_CAR_WARNING);
  // and again all stop
}
