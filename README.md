# simsim800
Simple SIM800 library to send stuff via GPRS

Not tested on actual MCU yet! Just using mock_main.c on local machine.

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
void (*handle_tx)(char* buffer,uint16_t buf_len, uint16_t timeout);
void (*handle_rx)(char* buffer,uint16_t buf_len, uint16_t timeout);
void (*handle_set_gpio_led)(uint8_t state); 
void (*handle_set_gpio_pwr)(uint8_t state);
void (*handle_delay_ms)(uint16_t time_ms);
```
and the library (in theory) will work just fine

## Work = in progress 
Please don't use it yet but feel free to feel inspired.
