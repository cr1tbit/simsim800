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

    //these pointers must  be populated before
    //actually using the object
    void (*handle_tx)(char*,uint16_t, uint16_t);
    void (*handle_rx)(char*,uint16_t, uint16_t);

    //these aren't implemented yet so not really i guess
    void (*handle_set_gpio_led)(uint8_t);
    void (*handle_set_gpio_pwr)(uint8_t);
    void (*handle_delay_ms)(uint16_t);

    //internal buffers
    char rx_buf[RX_BUF_SIZE];
    char tx_buf[RX_BUF_SIZE];
    uint16_t buf_len;
} sim800_t;

/***
 * flush command should set both internal RX buffer,
 * and SIM800 state to default. The \r is sent via 
 * UART to discard any trash that might've been sent
 * to the modem before.
 **/
void sim800_flush(sim800_t *sim800){
    static char *cr_string = "\r";
    if (sim800->handle_tx != 0){
        sim800->handle_tx(
            cr_string,
            strlen(cr_string),
            0xFFFF
        );
    }
    memset(sim800->rx_buf,0x00,sim800->buf_len);
}

/***
 * Should be called on the sim800 struct at the beginning
 **/
uint8_t sim800_init(sim800_t *sim800){
    sim800->state = err_unknown;
    sim800->credentials_APN = "internet";
    sim800->credentials_user = "";
    sim800->credentials_passwd = "";

    //Please let me go back to python :(
    sim800->handle_tx = 0;
    sim800->handle_rx = 0;
    sim800->handle_set_gpio_led = 0;
    sim800->handle_set_gpio_pwr = 0;
    sim800->handle_delay_ms = 0;

    sim800->buf_len=RX_BUF_SIZE;
    sim800_flush(sim800);
}

/***
 * Handle command formatting, then send it via uart handle.
 **/

uint8_t sim800_command(
    sim800_t *sim800, 
    const char *format,
    ...){
    if (sim800->handle_tx != 0){
        sim800_flush(sim800);
        va_list argp;
        va_start(argp, format);
        vsnprintf(sim800->tx_buf,sim800->buf_len,format, argp);
        va_end(argp);
        sim800->tx_buf[(sim800->buf_len)-1] = (char)0x00; //add ending guard, just in case
        sim800->handle_tx(
            (uint8_t *)sim800->tx_buf, 
            strlen(sim800->tx_buf),
            0xFFFF);
    }
}
/*
void sim800_receive(
    sim800_t *sim800,
    uint16_t timeout_ms
){
    const uint16_t receive_chunks = 10;
    uint16_t chunk_no;
    uint16_t rx_pointer = 0;
    for (chunk_no=0;chunk_no<receive_chunks;chunk_no++){
        
    }


    sim800->handle_rx(
        sim800->rx_buf,
        sim800->buf_len,
        1000
    );
    sim800->rx_buf[(sim800->buf_len)-1] = 0x00;
    printf("R:%s\n",sim800->rx_buf);
}
*/

void sim800_receive(
    sim800_t *sim800,
    uint16_t timeout_ms
){
    sim800->handle_rx(
        sim800->rx_buf,
        sim800->buf_len,
        1000
    );
    sim800->rx_buf[(sim800->buf_len)-1] = 0x00;
    printf("R:%s\n",sim800->rx_buf);
}

uint8_t sim800_receive_match_pattern(
    sim800_t *sim800,
    uint16_t timeout_ms,
    const char *pattern
)
{
    sim800_receive(sim800,1000);
    return (strstr(sim800->rx_buf,pattern) != NULL);
}

uint8_t sim800_query(
    sim800_t *sim800,
    const char *command,
    const char *success_pattern
){
    sim800_command(sim800,"%s\r",command);

    return sim800_receive_match_pattern(sim800, 1000,success_pattern);
}

sim800_state_t sim800_get_state(sim800_t *sim800){
    if (sim800_query(sim800,"AT","OK")!=0x01)
        return err_unresponsive;

    if (sim800_query(sim800,"AT+cpin?","READY")!=0x01)
        return err_no_sim;

    if (sim800_query(sim800,"AT+CGATT?","1")!=0x01)
        return ok_no_net;

    if (sim800_query(sim800,"AT+CIPSTATUS","IP ")!=0x01)
        return ok_gsm_mode;

    return ok_gprs_mode;
}

uint8_t sim800_reboot(sim800_t *sim800){
    sim800_query(sim800,"AT+CFUN=1,1","OK");
}


uint8_t sim800_gprs_get(
    sim800_t *sim800,
    char *rx_buf,
    uint16_t buf_len,
    char *succ_pattern
){
    sim800_query(sim800,"AT+SAPBR=3,1,\"APN\",\"internet\"","OK");
    sim800_query(sim800,"AT+SAPBR=1,1","OK");
    sim800_query(sim800,"AT+HTTPINIT","OK");
    sim800_command(
        sim800,
        "AT+HTTPPARA=\"URL\",\"%s\"",
        rx_buf
        );
    usleep(1000000);
    sim800_query(sim800,"AT+HTTPACTION=0","OK");
    sim800_flush(sim800);
    usleep(8000000);
    if (sim800_query(sim800,"AT+HTTPREAD",succ_pattern) != 0x01){
        return 0x00;
    }
    printf("\n***rx_buf CONENT ***\n%s\n",sim800->rx_buf);
    sim800_query(sim800,"AT+CIPSHUT","OK");
    return 0x01;
}

uint8_t sim800_get_resp(
    sim800_t *sim800,
    uint16_t timeout_ms,
    char *resp_buf,
    uint16_t resp_len
    ){
    //naive dumb method - receive data till timeout happens.
    sim800->handle_rx(
        (uint8_t *)resp_buf, 
        resp_len,
        timeout_ms);
}



