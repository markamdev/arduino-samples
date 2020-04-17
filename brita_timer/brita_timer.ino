// Application configuration section - set data appropriately

// pin number of first timer led
#define LOWER_LED 9
// pin number of last timer led
#define HIGHER_LED 12
// pin number of blinking activity led
#define ACTIVITY_LED 5
// pin number of led indicating empty timer
#define EMPTY_LED 7
// pin for reload button
#define BUTTON_PIN 2
// time for button debouncing (delay between press and setting state as PRESSED)
#define BUTTON_PRESS_TIMEOUT 200
// delay time for one timer step (one LED shining time) in seconds
#define TIMER_STEP_TIMEOUT 5


// DO NOT CHANGE anything below this line
// ======================================

// additional safecheck (do not allow incorrect LED's settings)
#if (HIGHER_LED <= LOWER_LED)
#error Invalid LEDs pins settings
#endif

// additional safecheck for activity LED pin
#if ((ACTIVITY_LED >= LOWER_LED && ACTIVITY_LED <= HIGHER_LED) || ACTIVITY_LED == BUTTON_PIN)
#error Invalid activity led pin settings
#endif

// additional safecheck for empty LED pin
#if ((EMPTY_LED >= LOWER_LED && EMPTY_LED <= HIGHER_LED) || EMPTY_LED == BUTTON_PIN)
#error Invalid empty led pin settings
#endif

// additional safech for activity and empty LED pin
#if (EMPTY_LED == ACTIVITY_LED)
#error Acivity and empty led cannot use same pin number
#endif

#define LED_COUNT (HIGHER_LED - LOWER_LED + 1)

// vars and consts for button debouncer
enum {
  BUTTON_RELEASED = 0,
  BUTTON_DEBOUNCING,
  BUTTON_PRESSED,
};
const long int BUTTON_DEBOUNCE_DELAY = BUTTON_PRESS_TIMEOUT;
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
const long int LEDS_TIMER_DELAY = TIMER_STEP_TIMEOUT * 1000L; //2s per one LED on "display"
const long int LEDS_LOADING_DELAY = 500;
const long int LEDS_BLINKING_DELAY = 150;
long int leds_loading_timestamp = 0;
long int leds_timer_timestamp = 0;
long int leds_blinking_timestamp = 0;
int leds_level = 0;
int leds_blinking_state = 0;

// vars and consts for activity LED controler
const long int ACTIVITY_BLINK_DELAY = 350;
long int activity_timestamp = 0;
int activity_state = LOW;

// vars and consts for empty timer marking
int empty_state = 1; // start with empty timer

// other vars
long int current_time = 0;

void setup() {
  Serial.begin(38400);
  pinMode(BUTTON_PIN, INPUT);
  for (int i = LOWER_LED; i <= HIGHER_LED; i++)
  {
    pinMode(i, OUTPUT);
  }
  pinMode(ACTIVITY_LED, OUTPUT);
  pinMode(EMPTY_LED, OUTPUT);

  // some variables initialization
  activity_timestamp = millis();
  leds_timer_timestamp = activity_timestamp; // initialize with same value
  old_leds_state = new_leds_state = LEDS_EMPTY;
  // set empty led to initial state (on)
  digitalWrite(EMPTY_LED, empty_state);
}

// functions forward declaration
void process_button();
void process_activity();
void process_leds();
void set_leds(unsigned int count);

void loop() {
  current_time = millis();
  process_button();

  // display LEDs controller code
  process_leds();

  process_activity();
}


inline void process_button()
{
  // button debouncer code
  button_pin_state = digitalRead(BUTTON_PIN);
  if (button_pin_state == LOW)
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

inline void process_leds()
{
  if (button_state == BUTTON_PRESSED)
  {
    if (old_leds_state != LEDS_LOADING && old_leds_state != LEDS_BLINKING)
    {
      new_leds_state = LEDS_LOADING;
      leds_loading_timestamp = current_time;
    }
  }
  else
  {
    if (empty_state == 1)
    {
      new_leds_state = LEDS_EMPTY;
      // be sure there's no partial timer reset
      leds_level = 0;
    }
    else
    {
      new_leds_state = LEDS_TIMER;
    }
  }

  switch (new_leds_state)
  {
    case LEDS_EMPTY:
      if (old_leds_state != new_leds_state)
      {
        set_leds(0);
      }
      break;
    case LEDS_LOADING:
      if ((current_time - leds_loading_timestamp) > LEDS_LOADING_DELAY)
      {
        if (leds_level < LED_COUNT)
        {
          // counter max not reached yed
          leds_level++;
          leds_loading_timestamp = current_time;
          set_leds(leds_level);
        }
        else
        {
          empty_state = 0;
          // set proper state of empty marker
          digitalWrite(EMPTY_LED, empty_state);
          new_leds_state = LEDS_BLINKING;
          leds_blinking_timestamp = current_time;
        }
      }
      break;
    case LEDS_TIMER:
      if (old_leds_state != LEDS_TIMER)
      {
        set_leds(leds_level);
        leds_timer_timestamp = current_time;
      }
      if ((current_time - leds_timer_timestamp) > LEDS_TIMER_DELAY)
      {
        leds_level--;
        if (leds_level < 0)
        {
          new_leds_state = LEDS_EMPTY;
          empty_state = 1;
          // set proper state of empty marker
          digitalWrite(EMPTY_LED, empty_state);
        }
        else
        {
          leds_timer_timestamp = current_time;
        }
        set_leds(leds_level);
      }
      break;
    case LEDS_BLINKING:
      if ((current_time - leds_blinking_timestamp) > LEDS_BLINKING_DELAY)
      {
        if (leds_blinking_state == 0)
        {
          leds_blinking_state = 1;
          set_leds(0);
        }
        else
        {
          leds_blinking_state = 0;
          set_leds(LED_COUNT);
        }
        leds_blinking_timestamp = current_time;
      }
      break;
    default:
      // error condition
      new_leds_state = LEDS_EMPTY;
      Serial.println("* -> LEDS_EMPTY");
  }

  old_leds_state = new_leds_state;
}

inline void process_activity()
{
  // activity LED should blink all the time
  if ((current_time - activity_timestamp) > ACTIVITY_BLINK_DELAY)
  {
    if (activity_state == LOW)
    {
      activity_state = HIGH;
    }
    else
    {
      activity_state = LOW;
    }
    // save state change timestamp
    activity_timestamp = current_time;
    // warn LED state changed - write to pin
    digitalWrite(ACTIVITY_LED, activity_state);
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
  for (i = LOWER_LED + count; i <= HIGHER_LED; ++i)
  {
    digitalWrite(i, LOW);
  }
}
