/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Version: V1.0
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2008 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <rt_misc.h>
#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include <RTL.h>  


#pragma import(__use_no_semihosting_swi)
                                                     // RTX kernel functions & defines

#define CTRL_Q     0x11                             // Control+Q character code
#define CTRL_S     0x13                             // Control+S character code
#define DEL        0x7F
#define BACKSPACE  0x08
#define CR         0x0D
#define LF         0x0A


static BIT sendstop = __FALSE;                    // flag: marks XOFF character
static char recbuf[256];
static U8 ridx = 0;
static U8 widx = 0;

OS_TID wr_task;
OS_TID rd_task;


struct __FILE {
  int handle;                 // Add whatever you need here 
};
FILE __stdout;
FILE __stdin;

/*----------------------------------------------------------------------------
  GetKey
  event driven GetKey function
 *---------------------------------------------------------------------------*/
int GetKey (void) {

  if (ridx == widx) {
    rd_task = os_tsk_self ();
    os_evt_clr (0x0100, rd_task);
    os_evt_wait_or (0x0100, 0xffff);              // wait till character received
  }

  return (recbuf[ridx++]);
}

/*----------------------------------------------------------------------------
 *      Line Editor
 *---------------------------------------------------------------------------*/
void getline (char *line, int n) {
  int  cnt = 0;
  char c;

  do {
    if ((c = GetKey ()) == CR)  c = LF;    /* read character                 */
    if (c == BACKSPACE  ||  c == DEL) {    /* process backspace              */
      if (cnt != 0)  {            
        cnt--;                             /* decrement count                */
        line--;                            /* and line pointer               */
        putchar (BACKSPACE);               /* echo backspace                 */
        putchar (' ');
        putchar (BACKSPACE);
      }
    }
    else if (c != CTRL_Q && c != CTRL_S) {   /* ignore Control S/Q             */
      putchar (*line = c);                 /* echo and store character       */
      line++;                              /* increment line pointer         */
      cnt++;                               /* and count                      */
    }
  } while (cnt < n - 1  &&  c != LF);      /* check limit and line feed      */
  *(line - 1) = 0;                         /* mark end of string             */
}
/*----------------------------------------------------------------------------
  SendChar
  event driven SendChar function
 *---------------------------------------------------------------------------*/
int SendChar (int c) {

  wr_task = os_tsk_self ();                       // identify task for serial interrupt
  os_evt_clr (0x0100, wr_task);
 
  if (c == '\n') {                                // expand new line character:
    USART1->DR = ('\r' & 0x1FF);
    USART1->CR1 |= USART_FLAG_TXE;		          // enable TX interrupt if nothing to send
    os_evt_wait_or (0x0100, 0xffff);              // wait till character transmited
  }
  USART1->DR = (c & 0x1FF);                       // transmit a character
  USART1->CR1 |= USART_FLAG_TXE;		          // enable TX interrupt if nothing to send

  os_evt_wait_or (0x0100, 0xffff);                // wait till character transmited

  return (c);                                     // return character: ANSI requirement
}




/*----------------------------------------------------------------------------
  USART1_IRQHandler
  Handles USART1 global interrupt request.
 *----------------------------------------------------------------------------*/
void USART1_IRQHandler (void) {
  volatile unsigned int  IIR;
           unsigned char c;

  IIR = USART1->SR;
  if (IIR & USART_FLAG_RXNE) {                    // Receive interrupt
    USART1->SR &= ~USART_FLAG_RXNE;               // clear interrupt
    c = (char) (USART1->DR & 0x1FF);              // read character
    switch (c)  {                                 // process character
      case CTRL_S:
        sendstop = __TRUE;                        // if Control+S stop transmission
        break;

      case CTRL_Q:
        sendstop = __FALSE;                       // if Control+Q start transmission
        break;

      default:                                    // send character to a mailbox
        recbuf[widx++] = c;
        isr_evt_set (0x0100, rd_task);            // set event character received
        break;
    }
  }
                             
  if (IIR & USART_FLAG_TXE) {                     // Transmit interrupt
    USART1->SR  &= ~USART_FLAG_TXE;               // clear interrupt
    USART1->CR1 &= ~USART_FLAG_TXE;               // disable TX interrupt

    if (!sendstop)  {                             // if not Control+S received
      isr_evt_set (0x0100, wr_task);              // Go on, set event transmit ready
    }
  }

}



/*----------------------------------------------------------------------------
  fputc
 *----------------------------------------------------------------------------*/
int fputc(int ch, FILE *f) {
  return (SendChar(ch));
}

/*----------------------------------------------------------------------------
  fgetc
 *----------------------------------------------------------------------------*/
int fgetc(FILE *f) {
  return (SendChar(GetKey()));
}

/*----------------------------------------------------------------------------
  _ttywrch
 *----------------------------------------------------------------------------*/
void _ttywrch(int ch) {
 SendChar (ch);
}

/*----------------------------------------------------------------------------
  ferror
 *----------------------------------------------------------------------------*/
int ferror(FILE *f) {
                              // Your implementation of ferror
  return EOF;
}

/*----------------------------------------------------------------------------
  _sys_exit
 *----------------------------------------------------------------------------*/
void _sys_exit(int return_code) {
label:  goto label;           // endless loop
}
