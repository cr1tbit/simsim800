# simsim800
Simple SIM800 library to send stuff via GPRS

Barely usefull at all yet! Just hacked together some code because I really need to get 
my current project over with. But it works fine for me.

## The idea
This library is meant to be simple and portable. I personally don't really need 
SMS/calling/TCPsocket stuff. In my opinion the easiest way to send (low amounts)
of data with SIM800 module is by GET request, with URL like this:
http://example.com/api?payload=HERE_COMES_THE_PAYLOAD.

This method has been proven to be quite strudy in my previous project.

For more info I suggest checking this [link.](https://m2msupport.net/m2msupport/athttppara-set-paramaters-for-http-connection/)

## How do I
**Implement** and then **attach** your own variants of these functions (just like in mock_main.c):

```
//returns number of TXed chars
uint16_t (*handle_tx)(char* buffer,uint16_t buf_len, uint16_t timeout);
//returns number of RXed chars
uint16_t (*handle_rx)(char* buffer,uint16_t buf_len, uint16_t timeout);
uint8_t (*handle_set_gpio_led)((uint8_t state);
uint8_t (*handle_set_gpio_pwr)((uint8_t state);
uint8_t (*handle_delay_ms)(uint16_t time_ms);
//may be passed stub function on some UART implementations
void (*handle_flush)(); 
```
and the library (in theory) will work just fine

## Example snippet from my actual project
This code was deployed on STM32 Mcu with freeRTOS.

```
//Functions below utulize STM32CubeHal's uart methods.
uint16_t gsm_handle_rx(char* buffer,uint16_t len, uint16_t timeout){
  volatile uint16_t rx_test = 
    HAL_UART_Receive(&huart1, (uint8_t *)buffer, len-1, timeout);
  HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), timeout);
  return strlen(buffer);
}

uint16_t gsm_handle_tx(char* buffer,uint16_t len, uint16_t timeout){
  HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, timeout);
  return HAL_UART_Transmit(&huart1, (uint8_t *)buffer, len, timeout);
}

//This handle allows the library to use the custom delay method defined
//by freeRTOS - allowing other threads to run during wait.
uint8_t gsm_handle_delay_ms(uint16_t time_ms){
  osDelay(time_ms);
}

//This handle was not needed on linux implementation, but it is on STM32
//as UART peripheral may lock itself in some situations until the interrupt
//flag is cleared manually.
void gsm_flush(){
  __HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_NEF|UART_CLEAR_OREF);
  return;
}

uint8_t gsm_handle_gpio_pwr(uint8_t target_state){
  //not used
}

#define ADDR_STRING "http://example.com/?d="

uint8_t send_data(sim800_t *sim, char *data_str, int data_len){
  static int maxlen = 512;
  char addr[512];
  snprintf(addr,maxlen,ADDR_STRING"%s", data_str);

  return sim800_gprs_get(
      sim,
      addr,
      strlen(addr),
      "SUCC"//if the response from server equals this, the transfer completed succesfully.
  );
}

void gsmTask(void const *argument)
{
  osEvent e;
  t_printBlockMail *receivedMail;

  osSemaphoreDef(PRNT_EVT_SEM);
  printCpltSemaphore =
      osSemaphoreCreate(osSemaphore(PRNT_EVT_SEM), 1);

  //initialize and attach all sim800 - related bloat
  sim800_t sim;
  sim.handle_tx = &gsm_handle_tx;
  sim.handle_rx = &gsm_handle_rx;
  sim.handle_flush = &gsm_flush;
  sim.handle_set_gpio_led = &gsm_handle_gpio_pwr; //just
  sim.handle_set_gpio_pwr = &gsm_handle_gpio_pwr; //placeholders
  sim.handle_delay_ms = &gsm_handle_delay_ms;
  sim800_init(&sim);
  sim800_turn_on(&sim);

  while (1){
    ...
```


## Work = in progress 
The library is very simple but kind of works - feel free to try to use it.

Copyright Jakub Sadowski *ΞΔ*-Flavoured MIT license
