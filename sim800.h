//#include "sim800.c"
#define SIM_IN_BUF_SIZE 100
#include <string.h>
#include <stdint.h>
#include <stdbool.h>



typedef enum {
    err_unknown,
    err_unresponsive,
    err_no_sim,
    ok_no_net,
    ok_gsm_mode,
    ok_gprs_mode
} sim800_state_t;

typedef struct {
    sim800_state_t state;

    char *credentials_APN; 
    char *credentials_user;
    char *credentials_passwd;

    void (*uart_handle);
    void (*gpio_led_handle);
    void (*gpio_pwr_handle);

    void (*delay_function_handle);
} sim800_t;


uint8_t sim800_init(const sim800_t* sim800);
/*
uint8_t sim800_setup_credentials(
    const struct sim800_t,
    string APN,
    string user,
    string passwd
)
*/

uint8_t sim800_command(
    const sim800_t *sim800, 
    const char *format,
    ...
    );

sim800_state_t sim800_get_state(const sim800_t* sim800);



//this method waits for the complete message to appear
//in receive buffer. 
//If the message has not been completed before
//timeout happens, or buffer is overflown,
// 0x00 is returned.
//on exit the internal buffer is cleared.
uint8_t sim800_get_resp(
    const sim800_t* sim800,
    uint16_t timeout_ms,
    char *resp_buf,
    uint16_t resp_len
    );

//similar to above, but works in-place - 
//returns true if pattern is found, oterwise false.
//on exit the internal buffer is cleared.
uint8_t sim800_is_pattern_in_resp(
    const sim800_t *sim800,
    uint16_t timeout_ms,
    const char *pattern
);

void sim800_clear_buffer(const sim800_t *sim800);


void sim800_power_pulse(const sim800_t *sim800);
uint8_t sim800_gprs_enable(const sim800_t *sim800);
uint8_t sim800_gprs_disable(const sim800_t *sim800);


uint8_t sim800_gprs_get(
    const sim800_t *sim800,
    char *request,
    char *resp_buf,
    uint16_t resp_len
    );

