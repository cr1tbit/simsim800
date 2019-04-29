#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>


typedef enum {
    err_unknown = 0,
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

    void (*handle_tx)(char*,uint16_t, uint16_t);
    void (*handle_rx)(char*,uint16_t, uint16_t);
    void (*handle_set_gpio_led)(uint8_t);
    void (*handle_set_gpio_pwr)(uint8_t);
    void (*handle_delay_ms)(uint16_t);

    void (*delay_function_handle);

    char buffer[128];
    uint16_t buf_len;
} sim800_t;

void sim800_flush(sim800_t *sim800){
    //in in device buffer anuther command has been requested, yeet it out.
    static char *cr_string = "\r";
    if (sim800->handle_tx != 0){
        sim800->handle_tx(
            cr_string,
            strlen(cr_string),
            0xFFFF
        );
    }
    memset(sim800->buffer,0x00,sim800->buf_len);
}

uint8_t sim800_command(
    sim800_t *sim800, 
    const char *format,
    ...){
    static char _buf[100];
    if (sim800->handle_tx != 0){
        sim800_flush(sim800);
        va_list argp;
        va_start(argp, format);
        vsnprintf(_buf,100,format, argp);
        va_end(argp);
        _buf[99] = (char)0x00; //add ending guard, just in case
        //int i;
        //for (i=0;i<strlen(_buf);i++)
        //    printf("->%02x\n",_buf[i]);
        sim800->handle_tx(
            (uint8_t *)_buf, 
            strlen(_buf),
            0xFFFF);
    }
}



uint8_t sim800_init(sim800_t *sim800){
    sim800->state = err_unknown;
    sim800->credentials_APN = "internet";
    sim800->credentials_user = "";
    sim800->credentials_passwd = "";

    sim800->handle_tx = 0;
    sim800->handle_rx = 0;
    sim800->handle_set_gpio_led = 0;
    sim800->handle_set_gpio_pwr = 0;
    sim800->handle_delay_ms = 0;

    sim800->buf_len=128;
    sim800_flush(sim800);
}

void sim800_receive(
    sim800_t *sim800,
    uint16_t timeout_ms
){
    sim800->handle_rx(
        sim800->buffer,
        sim800->buf_len,
        1000
    );
    sim800->buffer[(sim800->buf_len)-1] = 0x00;
    //printf("buffer content after receive: \n%s\n",sim800->buffer);
    printf("R:%s\n",sim800->buffer);
}

uint8_t sim800_receive_match_pattern(
    sim800_t *sim800,
    uint16_t timeout_ms,
    const char *pattern
)
{
    sim800_receive(sim800,1000);
    return (strstr(sim800->buffer,pattern) != NULL);
}

uint8_t sim800_query(
    sim800_t *sim800,
    const char *command,
    const char *success_pattern
){
    sim800_command(sim800,"%s\r",command);

    if (sim800_receive_match_pattern(sim800, 1000,success_pattern) == 0x00)
        return 0x00;
    return 0x01;
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
    char *buffer,
    uint16_t buf_len
){
    sim800_query(sim800,"AT+SAPBR=3,1,\"APN\",\"internet\"","OK");
    //sim800_query(sim800,"AT+SAPBR=1,1","OK");
    //sim800_query(sim800,"AT+HTTPINIT","OK");
    sim800_command(
        sim800,
        "AT+HTTPPARA=\"URL\",\"%s\"",
        buffer
        );
    sim800_query(sim800,"AT+HTTPACTION=0","OK");
    sim800_flush(sim800);
    usleep(3000000);
    sim800_query(sim800,"AT+HTTPREAD","OK");
    printf("\n***BUFFER CONENT ***\n%s\n",sim800->buffer);
    sim800_query(sim800,"AT+CIPSHUT","OK");
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



