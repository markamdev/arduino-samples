// PINs' numbers settings
#define LOWER_LED 9
#define HIGHER_LED 12
#define WARN_LED 7
#define BUTTON_PIN 3

// additional safecheck (do not allow incorrect LED's settings)
#if (HIGHER_LED <= LOWER_LED)
#error Invalid LEDs pins settings
#endif
// additional safecheck for warning LED pin
#if ((WARN_LED >= LOWER_LED && WARN_LED <= HIGHER_LED) || WARN_LED == BUTTON_PIN)
#error Invalid WARN led pin settings
#endif

#define LED_COUNT (HIGHER_LED - LOWER_LED + 1)

// vars and consts for button debouncer
enum {
  BUTTON_RELEASED = 0,
  BUTTON_DEBOUNCING,
  BUTTON_PRESSED,
};
const long int BUTTON_DEBOUNCE_DELAY = 100;
int button_state = BUTTON_RELEASED;
long int button_press_timestamp = 0;
int button_pin_state = LOW;

// vars and consts for LED counter controler
enum {
  LEDS_TIMER = 0,
  LEDS_LOADING,
  LEDS_BLINKING,
  LEDS_EMPTY,
};
int new_leds_state = LEDS_EMPTY;
int old_leds_state = LEDS_EMPTY;
const long int LEDS_TIMER_DELAY = 10000;
const long int LEDS_LOADING_DELAY = 1000;
long int leds_timestamp = 0;
int leds_level = 0;

// vars and consts for warning LED controler
const long int WARN_BLINK_DELAY = 500;
long int warn_timestamp = 0;
int warn_state = LOW;

// other vars
long int current_time = 0;

void setup() {
  Serial.begin(38400);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  for (int i = LOWER_LED; i <= HIGHER_LED; i++)
  {
    pinMode(i, OUTPUT);
  }
  pinMode(WARN_LED, OUTPUT);

  // some variables init
  warn_timestamp = millis();
  leds_timestamp = warn_timestamp; // initialize with same value
}

// functions forward declaration
void process_button();
void process_warning();
void process_leds();

void loop() {
  current_time = millis();
  process_button();

  // display LEDs controller code
  process_leds();

  process_warning();
}


inline void process_button()
{
  // button debouncer code
  button_pin_state = digitalRead(BUTTON_PIN);
  if (button_pin_state == HIGH)
  {
    if (button_state == BUTTON_PRESSED)
    {
      Serial.println("BUTTON_PRESSED -> BUTTON_RELEASED");
    }
    button_state = BUTTON_RELEASED;
  }
  else
  {
    // already in debouncing state
    switch (button_state)
    {
      case BUTTON_RELEASED:
        button_state = BUTTON_DEBOUNCING;
        // re-use already fetched time
        button_press_timestamp = current_time;
        break;
      case BUTTON_DEBOUNCING:
        // additional check for overflow
        if (current_time < button_press_timestamp)
        {
          button_press_timestamp = current_time;
        }
        if ((current_time - button_press_timestamp) > BUTTON_DEBOUNCE_DELAY)
        {
          button_state = BUTTON_PRESSED;
          Serial.println("BUTTON_DEBOUNCING -> BUTTON_PRESSED");
        }
        break;
      case BUTTON_PRESSED:
        // already pressed - do nothing
        break;
      default:
        // some error occured
        Serial.println("* -> BUTTON_RELEASED");
        button_state = BUTTON_RELEASED;
    }

  }
}

void set_leds(unsigned int count)
{
  if (count > LED_COUNT)
  {
    return;
  }
  // turn on X leds at beginning
  int i  = 0;
  for (i = 0; i < count; ++i)
  {
    digitalWrite(LOWER_LED + i, HIGH);
  }
  // turn off rest of leds
  for (i = LOWER_LED + count; i < HIGHER_LED; ++i)
  {
    digitalWrite(i, LOW);
  }
}

inline void process_leds()
{
  if (button_state == BUTTON_PRESSED)
  {
    new_leds_state = LEDS_LOADING;
  }
  else
  {
    if (leds_level > 0)
    {
      new_leds_state = LEDS_TIMER;
    }
    else
    {
      new_leds_state = LEDS_EMPTY;
    }
  }

  if (new_leds_state != old_leds_state)
  {
    switch (new_leds_state)
    {
      case LEDS_EMPTY:
        set_leds(0);
        break;
      case LEDS_LOADING:
        set_leds(2);
        break;
      case LEDS_TIMER:
        break;
      case LEDS_BLINKING:
        break;
      default:
        // error condition
        new_leds_state = LEDS_EMPTY;
        Serial.println("* -> LEDS_EMPTY");
    }
    old_leds_state = new_leds_state;
  }
}

inline void process_warning()
{
  // warning (LED) controller code
  if (button_state != BUTTON_PRESSED)
  {
    // warning LED should blink only if button not fully pressed
    if ((current_time - warn_timestamp) > WARN_BLINK_DELAY)
    {
      if (warn_state == LOW)
      {
        warn_state = HIGH;
      }
      else
      {
        warn_state = LOW;
      }
      // save state change timestamp
      warn_timestamp = current_time;
      // warn LED state changed - write to pin
      digitalWrite(WARN_LED, warn_state);
    }
  }
}
