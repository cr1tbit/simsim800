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



int fail_handle(){
    printf("Tetss failed. Exiting...");
    return -1;
}

/**** mock-based testing ****/

void mock_tx(char* buffer,uint16_t len, uint16_t timeout){
    printf("TX order: %s\n",buffer);
};
void mock_rx(char* buffer,uint16_t len, uint16_t timeout){
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

void uart_tx(char* buffer,uint16_t len, uint16_t timeout){
    printf("TXing: %s\n",buffer);
    if (write(fd, buffer, len) < len)
        printf("possible fuckup writing to port.\n");
};
void uart_rx(char* buffer,uint16_t len, uint16_t timeout){
    printf("RXing started...\n");
    usleep(1000000);
    int _bno = read(fd, buffer,len); 
    if (_bno <=0)
        printf("possible fuckup reading from port.\n");
    printf("RXed %d bytes: \n%s\n",_bno,buffer);
}

int test_real_uart(sim800_t *sim){
    printf("\n\n*** REAL UART TEST ***\n");
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1){
        printf("fucked opening port.");
        return -1;
    }
    sim->handle_tx = &uart_tx;
    sim->handle_rx = &uart_rx;
    
    if (sim800_query(sim,"AT","OK") !=0x01)
        return fail_handle();
    if (sim800_query(sim,"AT","NOTOK") !=0x00)
        return fail_handle();
    return 0;
}


int main( int argc, const char* argv[] )
{
	printf( "\nRunning mock HAL thingy\n\n" );

    sim800_t sim;
    sim800_init(&sim);

    //if (test_mock_uart(&sim) == -1)
    //    return -1;
    
    if (test_real_uart(&sim) == -1)
        return -1;

    printf("detected state: %d\n",sim800_get_state(&sim));
    //sim800_command(&sim,"AT+CSTT=\"%s\"",sim.credentials_APN);
    usleep(300000);

    char *addr = "http://qrng.anu.edu.au/API/jsonI.php?length=1&type=uint8";

    sim800_gprs_get(
        &sim,
        addr,
        strlen(addr)
    );
}

