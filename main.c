#ifndef _ATMEL_H
#define _ATMEL_H
#include <AT91SAM9263.h>
#endif

#define BAUDRATE 115200
#define MCK 100 * 1000000
#define BAUDRATE_GENERATOR_DIVIDER 16
#define GDBU_BAUD_RATE_GENERATOR_CD_FOR_115200BPS MCK / (BAUDRATE_GENERATOR_DIVIDER * BAUDRATE)

#define DISTANECE_FROM_LOWER_TO_CAPITAL 'a' - 'A'

#define FIFO_SIZE 16
#define FIFO_BUFFOR_SIZE FIFO_SIZE + 1

void disable_all_DBGU_interrupts()
{
  AT91C_BASE_DBGU->DBGU_IDR = AT91C_US_RXRDY | AT91C_US_TXRDY | AT91C_US_ENDRX | AT91C_US_ENDTX | AT91C_US_OVRE | AT91C_US_FRAME |
			      AT91C_US_PARE | AT91C_US_TXEMPTY | AT91C_US_TXBUFE | AT91C_US_RXBUFF | AT91C_US_COMM_TX | AT91C_US_COMM_RX;
}

void turn_off_and_reset_reciver()
{
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXDIS | AT91C_US_RSTRX;
}

void turn_off_and_reset_transmitter()
{
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_TXDIS | AT91C_US_RSTTX;
}

void init_PIO_ports()
{
  AT91C_BASE_PIOC->PIO_ASR = AT91C_PC30_DRXD | AT91C_PC31_DTXD;
  AT91C_BASE_PIOC->PIO_PDR = AT91C_PC30_DRXD | AT91C_PC31_DTXD;
}

void configure_throughput()
{
  AT91C_BASE_DBGU->DBGU_BRGR = GDBU_BAUD_RATE_GENERATOR_CD_FOR_115200BPS;
}

void configure_operation_mode()
{
  AT91C_BASE_DBGU->DBGU_MR = AT91C_US_CHMODE_NORMAL | AT91C_US_PAR_NONE;
}

void Open_DBGU_INT()
{
  
}

void turn_on_receiver()
{
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXEN;
}

void turn_on_transmitter()
{
 AT91C_BASE_DBGU->DBGU_CR = AT91C_US_TXEN;
}

void Open_DBGU()
{
  disable_all_DBGU_interrupts();
  turn_off_and_reset_reciver();
  turn_off_and_reset_transmitter();
  init_PIO_ports();
  configure_throughput();
  configure_operation_mode();
  turn_on_receiver();
  turn_on_transmitter();
  Open_DBGU_INT();
}

void send_char(char character)
{
  while(!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY))
    ;
  AT91C_BASE_DBGU->DBGU_THR = (unsigned char)character;
}

char get_char()
{
  while(!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY))
    ;
  return (unsigned char)AT91C_BASE_DBGU->DBGU_RHR;
}

char capitalize(char character)
{
  if (character >= 'A' && character <= 'Z')
    return character += DISTANECE_FROM_LOWER_TO_CAPITAL;
  else if (character >= 'a' && character <= 'z')
    return character -= DISTANECE_FROM_LOWER_TO_CAPITAL;
  else
    return 0;
}

void print_alphabet()
{
  unsigned char letter;
  for(letter = 'a'; letter <= 'z'; ++letter)
    send_char(letter);
}

void printf(char* string)
{
    while(*string)
      send_char(*(string++));
}

typedef struct FIFO
{
  char storage[FIFO_BUFFOR_SIZE];
  char* head;
  char* tail;
} FIFO;

void FIFO_init(FIFO* fifo)
{
  fifo->head = fifo->storage;
  fifo->tail = fifo->storage;
}

void FIFO_Empty(FIFO* fifo)
{
  fifo->tail = fifo->head;
}

int FIFO_is_empty(FIFO* fifo)
{
  return fifo->head == fifo->tail;
}

int FIFO_Get(FIFO* fifo, char* data)
{
  if (FIFO_is_empty(fifo))
    return 1;
  *data = *(fifo->tail++);
  if (fifo->tail - fifo->storage >= FIFO_BUFFOR_SIZE)
    fifo->tail = fifo->storage;
  return 0;
}

int FIFO_Put(FIFO* fifo, char data)
{
  if (fifo->tail - fifo->head == 1 || (fifo->tail == fifo->storage && fifo->head == &(fifo->storage[FIFO_SIZE])))
    return 1;
  *(fifo->head++) = data;
  if (fifo->head - fifo->storage>= FIFO_BUFFOR_SIZE)
    fifo->head = fifo->storage;
  return 0;
}

void FIFO_print(FIFO* fifo)
{
  char to_print;
  while(!FIFO_Get(fifo, &to_print))
    send_char(to_print);
  printf("\n\r");
}

void dbgu_print_ascii(char* const text)
{
  printf(text);
}


int main()
{
  Open_DBGU();
  print_alphabet();
  printf("\n\rHello World!\n\r");

   FIFO fifo;
   FIFO_init(&fifo);

  char recived_char;
  while(1) {
    recived_char = get_char();
    if (recived_char == '\r')
      FIFO_print(&fifo);
    else if (FIFO_Put(&fifo, recived_char))
    {
      FIFO_print(&fifo);
      printf("FIFO overflow\n\r");
    }
  }
}