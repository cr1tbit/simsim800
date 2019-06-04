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

## Work = in progress 
Please don't use it yet but feel free to feel inspired.

In order for this lib to make sense, the handlers magic should be rewritten into
a lot of #DEFINE magic. One day.
