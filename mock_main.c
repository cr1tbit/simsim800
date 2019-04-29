#include <stdint.h>

//void mock_tx(char*,uint16_t, uint16_t);
//void mock_rx(char*,uint16_t, uint16_t);

#include "sim800.c"
#include <stdio.h>

#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define EMPLOYER_FRIENDLY_PHRASE "screwup"


int fail_handle(){
    printf("Tetss failed. Exiting...");
    return -1;
}

/**** mock-based testing ****/

uint16_t mock_tx(char* buffer,uint16_t len, uint16_t timeout){
    printf("TX order: %s\n",buffer);
};
uint16_t mock_rx(char* buffer,uint16_t len, uint16_t timeout){
    printf("putting mock OK in buffer\n");
    snprintf(buffer,128,"OK");
}

int test_mock_uart(sim800_t *sim){
    printf("\n\n*** MOCK UART TEST ***\n");
    sim->handle_tx = &mock_tx;
    sim->handle_rx = &mock_rx;

    if (sim800_query(sim,"AT\r","OK") !=0x01)
        return fail_handle();
    if (sim800_query(sim,"AT\r","NOTOK") !=0x00)
        return fail_handle();
    return 0;
}

/*** real shit ****/

int fd;

//#define MOCK_UART_DEBUG

uint16_t uart_tx(char* buffer,uint16_t len, uint16_t timeout){
    //#ifdef MOCK_UART_DEBUG
        printf("Q: %s\n",buffer);
    //#endif
    if (write(fd, buffer, len) < len)
        printf("possible "EMPLOYER_FRIENDLY_PHRASE" writing to port.\n");
};
uint16_t uart_rx(char* buffer,uint16_t len, uint16_t timeout){
    #ifdef MOCK_UART_DEBUG
        printf("RXing started...\n");
    #endif
    usleep(timeout*1000);
    int _bno = read(fd, buffer,len); 
    if (_bno <=0)
        printf("possible "EMPLOYER_FRIENDLY_PHRASE" reading from port.\n");
    #ifdef MOCK_UART_DEBUG
        printf("RXed %d bytes: \n%s\n",_bno,buffer);
    #endif
}

uint8_t unix_delay_ms(uint16_t time_ms){
    usleep(time_ms*1000);
}

int test_real_uart(sim800_t *sim){
    printf("\n\n*** REAL UART TEST ***\n");    
    if (sim800_query(sim,"AT","OK") !=0x01)
        return fail_handle();
    if (sim800_query(sim,"AT","NOTOK") !=0x00)
        return fail_handle();
    return 0;
}

uint8_t send_data(sim800_t *sim){
    char *addr = "http://qrng.anu.edu.au/API/jsonI.php?length=1&type=uint8";
    
    return sim800_gprs_get(
        sim,
        addr,
        strlen(addr),
        "\"success\""
    );
}

int main( int argc, const char* argv[] )
{
	printf( "\n Running mock HAL thingy\n\n" );

    sim800_t sim;
    sim800_init(&sim);

    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1){
        printf(EMPLOYER_FRIENDLY_PHRASE" opening port.");
        return -1;
    }
    sim.handle_tx = &uart_tx;
    sim.handle_rx = &uart_rx;
    sim.handle_delay_ms = &unix_delay_ms;
    int i;


    for (i=0;i<5;i++){
        if (sim800_get_state(&sim) == 5){
            printf("Valid state detected, doing the send thing...");
            if (send_data(&sim)==0x01)
                break;
        }
        usleep(3000000);
    }
    //sim800_command(&sim,"AT+CSTT=\"%s\"",sim.credentials_APN);
}

