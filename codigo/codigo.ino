#define LED_ROJO 2
#define POT 34
#define BTN 15

volatile int ticks = 0;
volatile int counter = 0;
volatile int lockBtn = 0;

int estado = 0;
int pwm = 0;
int parpadeos = 0;
bool ledState = 0;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  ticks = 1;
  counter++;

  if (lockBtn > 0) lockBtn--;

  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR isrBtn()
{
  if (lockBtn == 0)
  {
    estado++;
    if (estado > 2) estado = 0;
    lockBtn = 1;
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);

  attachInterrupt(BTN, isrBtn, FALLING);
  ledcAttach(LED_ROJO, 5000, 8);

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);
}

void loop()
{
  if (ticks == 1)
  {
    ticks = 0;

    if (estado == 0)
    {
      Serial.println("Estado 0 - Inactivo");
      ledcWrite(LED_ROJO, 0);
      counter = 0;
      parpadeos = 0;
      ledState = 0;
    }
    else if (estado == 1)
    {
      int lectura = analogRead(POT);
      pwm = map(lectura, 0, 4095, 0, 255);

      Serial.print("Estado 1 (Potenciometro) - PWM: ");
      Serial.println(pwm);
      ledcWrite(LED_ROJO, pwm);
    }
    else if (estado == 2)
    {
      ledState = !ledState;
      ledcWrite(LED_ROJO, ledState ? 255 : 0);

      if (ledState)
      {
        parpadeos++;
        Serial.print("Estado 2 - Parpadeo: ");
        Serial.println(parpadeos);
      }

      if (parpadeos >= 5 && !ledState)
      {
        Serial.println("Fin parpadeo, regreso a estado 0");
        ledcWrite(LED_ROJO, 0);
        estado = 0;
        parpadeos = 0;
        counter = 0;
      }
    }
  }
}