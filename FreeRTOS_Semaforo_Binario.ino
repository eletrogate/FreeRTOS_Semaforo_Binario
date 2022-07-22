/**
 * @file main.cpp
 * @author Evandro Teixeira
 * @brief 
 * @version 0.1
 * @date 21-01-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
/* https://github.com/RobTillaart/PCF8574 */
#include "PCF8574.h"  

#define COLOR_BLACK       "\e[0;30m"
#define COLOR_RED         "\e[0;31m"
#define COLOR_GREEN       "\e[0;32m"
#define COLOR_YELLOW      "\e[0;33m"
#define COLOR_BLUE        "\e[0;34m"
#define COLOR_PURPLE      "\e[0;35m"
#define COLOR_CYAN        "\e[0;36m"
#define COLOR_WRITE       "\e[0;37m"
#define COLOR_RESET       "\e[0m"
#define TASK_DELAY                1000
#define PRIMARY_CORE              PRO_CPU_NUM
#define SECONDARY_CORE            APP_CPU_NUM
#define ENABLE                    1
#define DISABLE                   0
#define ENABLE_SEMAPHORE_SERIAL   DISABLE 
#define PIN_IRQ_EXP_IO            15 
#define ADDRESS_I2C_EXPIO         0x20
#define DEBONCE_IRQ               500

/**
 * @brief 
 */
void Tarefa_A(void *parameters);
void Tarefa_B(void *parameters);
void Tarefa_ExpIO(void *parameters);

/**
 * @brief 
 */
const char txt[] = {"Contribua com a comunidade Maker"};
const char txt1[] = {"Torne-se um Eletrogater Expert"};

/**
 * @brief 
 */
SemaphoreHandle_t xSemaphore_Serial = NULL;
SemaphoreHandle_t xSemaphore_ExpIO = NULL;

/**
 * @brief Cria objeto PCF8574
 */
PCF8574 ExpIO(ADDRESS_I2C_EXPIO);

/**
 * @brief 
 */
void IRAM_ATTR ExpIO_ISR()
{
  static uint32_t last_time = 0;
  if((millis() - last_time) >= DEBONCE_IRQ)
  {
#if ENABLE_SEMAPHORE_SERIAL
    xSemaphoreTakeFromISR(xSemaphore_Serial,pdFALSE);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    Serial.printf("\n\rIRQ Libera o Semaforo Binario xSemaphore_ExpIO");

#if ENABLE_SEMAPHORE_SERIAL
    xSemaphoreGiveFromISR(xSemaphore_Serial,pdFALSE);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    last_time = millis();
    xSemaphoreGiveFromISR(xSemaphore_ExpIO, (BaseType_t)(pdFALSE));
  }
}

/**
 * @brief 
 * 
 */
void setup() 
{
  // Inicializa a Serial 
  Serial.begin(115200);
  Serial.printf("\n\rFreeRTOS - Semafaro Binario \n\r");

  // Inicializa pino 15 como entra e inicializa interrupção 
  pinMode(PIN_IRQ_EXP_IO, INPUT_PULLUP);
  attachInterrupt(PIN_IRQ_EXP_IO, ExpIO_ISR, FALLING);

  // Inicializar PCF8574
  ExpIO.begin();

#if ENABLE_SEMAPHORE_SERIAL
  // Cria semafaro binario xSemaphore_Serial  
  vSemaphoreCreateBinary( xSemaphore_Serial );
  if(xSemaphore_Serial == NULL)
  {
    Serial.printf("\n\rFalha em criar o semafaro para Serial");
  }
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  // Cria semafaro binario xSemaphore_ExpIO
  vSemaphoreCreateBinary( xSemaphore_ExpIO );
  if(xSemaphore_ExpIO == NULL)
  {
    Serial.printf("\n\rFalha em criar o semafaro para Expansor I/O");
  }
  // Obtem o semafaro xSemaphore_ExpIO
  xSemaphoreTake(xSemaphore_ExpIO,(TickType_t)100);

  // Cria tarefas da aplicação
  xTaskCreatePinnedToCore(Tarefa_A, "Tarefa_A",  configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 1, NULL, SECONDARY_CORE);
  xTaskCreatePinnedToCore(Tarefa_B, "Tarefa_B",  configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 1, NULL, PRIMARY_CORE);
  xTaskCreatePinnedToCore(Tarefa_ExpIO, "Tarefa_ExpIO",  configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 2, NULL, tskNO_AFFINITY);

}
/**
 * @brief 
 * 
 */
void loop() 
{

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  Serial.printf("\n\rTarefa Loop");
  vTaskDelay((TASK_DELAY/5)/portTICK_PERIOD_MS);

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */
}

/**
 * @brief 
 * 
 * @param parameters 
 */
void Tarefa_A(void *parameters)
{

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  Serial.print(COLOR_RED);                      // altera para vermelho impressão da mensagem na serial
  Serial.printf("\n\r%s",pcTaskGetTaskName(NULL));
  Serial.printf("\n\r%s",txt);
  Serial.printf("\n\r%s",txt1);
  Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  while (1)
  {

#if ENABLE_SEMAPHORE_SERIAL 
    xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    Serial.print(COLOR_RED);                      // altera para vermelho impressão da mensagem na serial
    Serial.printf("\n\r%s",pcTaskGetTaskName(NULL));
    Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 

#if ENABLE_SEMAPHORE_SERIAL 
    xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    vTaskDelay((TASK_DELAY)/portTICK_PERIOD_MS);    // Pausa a execução da tarefa por 2000 milessegundos 
  }
}

/**
 * @brief 
 * 
 * @param parameters 
 */
void Tarefa_B(void *parameters)
{
  
#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  Serial.print(COLOR_YELLOW);                   // altera para vermelho impressão da mensagem na serial
  Serial.printf("\n\r%s",pcTaskGetTaskName(NULL));
  Serial.printf("\n\r%s",txt);
  Serial.printf("\n\r%s",txt1);
  Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  while (1)
  {

#if ENABLE_SEMAPHORE_SERIAL 
    xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    Serial.print(COLOR_YELLOW);                   // altera para vermelho impressão da mensagem na serial
    Serial.printf("\n\r%s",pcTaskGetTaskName(NULL));
    Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 

#if ENABLE_SEMAPHORE_SERIAL 
    xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    vTaskDelay(TASK_DELAY/portTICK_PERIOD_MS);    // Pausa a execução da tarefa por 2000 milessegundos 
  }
}

/**
 * @brief 
 * 
 * @param parameters 
 */
void Tarefa_ExpIO(void *parameters)
{
  uint8_t value = 0;

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  Serial.print(COLOR_PURPLE);                   // altera para roxo impressão da mensagem na serial
  Serial.printf("\n\r%s",pcTaskGetTaskName(NULL));
  Serial.printf("\n\r%s",txt);
  Serial.printf("\n\r%s",txt1);
  Serial.print(COLOR_RESET);                    // reset a cor da impressão da mensagem na serial 

#if ENABLE_SEMAPHORE_SERIAL 
  xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */

  while (1)
  {
    if(xSemaphoreTake(xSemaphore_ExpIO,portMAX_DELAY) == pdTRUE)
    {
      value = ExpIO.read8();
      
#if ENABLE_SEMAPHORE_SERIAL 
      xSemaphoreTake(xSemaphore_Serial,portMAX_DELAY);
#endif /* ENABLE_SEMAPHORE_SERIAL  */

      Serial.print(COLOR_PURPLE);                // altera para roxo impressão da mensagem na serial
      Serial.printf("\n\rPCF8574 Value: %d",value);
      Serial.print(COLOR_RESET);                 // reset a cor da impressão da mensagem na serial 

#if ENABLE_SEMAPHORE_SERIAL 
      xSemaphoreGive( xSemaphore_Serial );
#endif /* ENABLE_SEMAPHORE_SERIAL  */

    }
  }
}
