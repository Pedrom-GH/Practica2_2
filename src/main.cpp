#include <Arduino.h>

// Definición del pin del LED
const uint8_t PIN_LED = 2;

// Variables volátiles: se cargan desde RAM porque cambian en la ISR
volatile int interruptCounter;
int totalInterruptCounter;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Rutina de Servicio de Interrupción (ISR) para el Timer
void IRAM_ATTR onTimer() {
  // Entramos en una sección crítica para evitar conflictos de escritura
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(115200);

  // Configurar el pin del LED como salida
  pinMode(PIN_LED, OUTPUT);

  // 1. Iniciar timer 0, preescalador 80 (para contar microsegundos), contar hacia arriba
  // Frecuencia CPU 80MHz / 80 = 1MHz (1 tick = 1 microsegundo)
  timer = timerBegin(0, 80, true);
  
  // 2. Adjuntar la función onTimer a nuestro temporizador
  timerAttachInterrupt(timer, &onTimer, true);
  
  // 3. Establecer alarma cada 1,000,000 microsegundos (1 segundo)
  // true significa que el contador se reinicia automáticamente (autoreload)
  timerAlarmWrite(timer, 1000000, true);
  
  // 4. Habilitar la alarma
  timerAlarmEnable(timer);
}

void loop() {
  // Si la interrupción ha ocurrido (interruptCounter > 0)
  if (interruptCounter > 0) {

    // Sección crítica para decrementar el contador de forma segura
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    // Acción física: Conmutar el estado del LED (Parpadeo)
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));

    totalInterruptCounter++;

    Serial.print("An interrupt has occurred. Total number: ");
    Serial.println(totalInterruptCounter);
  }
}