#ifndef SIMSIM800_H
#define SIMSIM800_H

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define RX_BUF_SIZE 128


typedef enum {
    err_unknown = -1,
    ok_off = 0,
    err_unresponsive = 1,
    err_no_sim = 2,
    ok_no_net = 3,
    ok_gsm_mode = 4,
    ok_gprs_mode = 5
} sim800_state_t;

typedef struct {
    sim800_state_t state;

    char *credentials_APN; 
    char *credentials_user;
    char *credentials_passwd;

    /* 
     * these pointers must  be populated before
     * actually using the object
     */
    //returns number of TXed chars
    uint16_t (*handle_tx)(char* buffer,uint16_t buf_len, uint16_t timeout);
    //returns number of RXed chars
    uint16_t (*handle_rx)(char* buffer,uint16_t buf_len, uint16_t timeout);
    uint8_t (*handle_set_gpio_led)(uint8_t state);
    uint8_t (*handle_set_gpio_pwr)(uint8_t state);
    uint8_t (*handle_delay_ms)(uint16_t time_ms);

    //internal buffers
    char rx_buf[RX_BUF_SIZE];
    char tx_buf[RX_BUF_SIZE];
    uint16_t buf_len;
} sim800_t;

uint8_t sim800_command(sim800_t *sim800, const char *format, ...);
void sim800_flush(sim800_t *sim800);
uint8_t sim800_get_resp(sim800_t *sim800, uint16_t timeout_ms, char *resp_buf, uint16_t resp_len );
sim800_state_t sim800_get_state(sim800_t *sim800);
uint8_t sim800_gprs_get(sim800_t *sim800, char *rx_buf, uint16_t buf_len, char *succ_pattern );
uint8_t sim800_init(sim800_t *sim800);
void sim800_power_pulse(sim800_t *sim800);
uint8_t sim800_query(sim800_t *sim800, const char *command, const char *success_pattern );
uint8_t sim800_reboot(sim800_t *sim800);
uint8_t sim800_receive_match_pattern(sim800_t *sim800, uint16_t timeout_ms, const char *pattern );
void sim800_receive(sim800_t *sim800, uint16_t timeout_ms );
uint8_t sim800_turn_off(sim800_t *sim800);
uint8_t sim800_turn_on(sim800_t *sim800);


#endif /*SIMSIM800_H*/