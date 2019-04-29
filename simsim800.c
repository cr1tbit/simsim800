#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "simsim800.h"




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
    /*
    //Please let me go back to python :(
    sim800->handle_tx = 0;
    sim800->handle_rx = 0;
    sim800->handle_set_gpio_led = 0;
    sim800->handle_set_gpio_pwr = 0;
    sim800->handle_delay_ms = 0;
    */
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

void sim800_power_pulse(sim800_t *sim800){
    sim800->handle_set_gpio_pwr(1);
    sim800->handle_delay_ms(500);
    sim800->handle_set_gpio_pwr(0);
}

uint8_t sim800_turn_on(sim800_t *sim800){
    int i;
    for (i=0;i<3;i++){
        if (sim800_query(sim800,"AT","OK")==0x01)
            return 0x01;//modem is on - quit
        else
            sim800_power_pulse(sim800);
    }
    return 0x00;
}

uint8_t sim800_turn_off(sim800_t *sim800){
    int i;
    for (i=0;i<3;i++){
        if (sim800_query(sim800,"AT","OK")==0x00)
            return 0x01;//modem is not responding - assume it's off and quit
        else
            sim800_power_pulse(sim800);
    }
    return 0x00;
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
    sim800_query(sim800,"AT+SAPBR=1,1","");
    sim800_query(sim800,"AT+HTTPINIT","");
    sim800_command(
        sim800,
        "AT+HTTPPARA=\"URL\",\"%s\"",
        rx_buf
        );
    sim800->handle_delay_ms(1000);
    sim800_query(sim800,"AT+HTTPACTION=0","OK");
    sim800_flush(sim800);
    sim800->handle_delay_ms(8000);
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



