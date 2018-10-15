/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "LCD.h"


/* USER CODE BEGIN Includes */
#include <stdio.h>                    /* standard I/O .h-file                */
#include <ctype.h>                    /* character functions                 */
#include <string.h>                   /* string and memory functions         */
#include <stdlib.h>
#include <stdbool.h>
#include <stm32_hal_legacy.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ESC				0x1B				/* Caractère Escape */
#define CTRL_Q     0x11                             // Control+Q character code
#define CTRL_S     0x13                             // Control+S character code
#define DEL        0x7F
#define BACKSPACE  0x08
#define CR         0x0D
#define LF         0x0A


const char menu[] = 
 
   "\n"
   "+**************         CONTROLLEUR DE FEUX            ********+\n"
   "|                                                              |\n"
   "+ command -+ syntax -----+ function ---------------------------+\n"
   "| ESC      |             | entree commandes                    |\n"
   "|          |             |                                     |\n"
   "| Horloge  | H hh:mm:ss  | mise a jour horloge                 |\n"
   "| Debut    | D hh:mm:ss  | mise a jour debut controle feux     |\n"
   "| Fin      | F hh:mm:ss  | mise a jour fin controle feux       |\n"
   "+----------+-------------+-------------------------------------+\n";


struct temps {
	volatile unsigned int			 heure;
	volatile unsigned int			 min;
	volatile unsigned int			 sec;
	
};
const char * chaine1 = " Horloge: %02d: %02d: %02d ";
const char * chaine2 = " Debut: %02d:%02d:%02d  / ";
const char * chaine3 = " Fin: %02d:%02d:%02d  Control FEUX \r";

struct print_H {
	const char * chaine;
	unsigned int			 heure;
	unsigned int			 min;
	unsigned int			 sec;
};
	
struct print_H aff_H;

 struct temps debut = 	{ 6, 0, 0 };
 struct temps fin   = 	{ 18, 0, 0 };
 struct temps horloge = 	{ 12, 0, 0 };
struct temps v_temps;

volatile bool DPV1;
volatile bool DPV2;
volatile bool detect1;
volatile bool	detect2;
char ph;
char phtrain;
char task;
bool escape;
bool capteur;
bool modeManuel;
bool nextStep;
bool train;
/* USER CODE END Includes */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void controleur( void *pvParameters );
void command  (void *pvParameters);
void lecture_BP (void *pvParameters);

void sequenceur (bool);
void seqtrain (bool);
bool generation_temps ( void) ;
bool lect_H (char  * )	;
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
QueueHandle_t xRxQueue;
QueueHandle_t xTxQueue;
SemaphoreHandle_t semaph;
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
		xRxQueue = xQueueCreate( 16, sizeof( char ) );
		xTxQueue = xQueueCreate( 3, sizeof( struct print_H) );
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
	__HAL_UART_ENABLE_IT(  &huart1,UART_IT_RXNE );
	MX_TIM2_Init();
  MX_TIM3_Init();
  __HAL_TIM_SetCompare (&htim2, TIM_CHANNEL_3, 0);
	__HAL_TIM_SetCompare (&htim2, TIM_CHANNEL_4, 0);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	
	HAL_TIM_Base_Start_IT(&htim3);
	
	lcd_init ();
  /* USER CODE BEGIN 2 */
 xTaskCreate(      controleur,       /* Function that implements the task. */
                    "CONTROLEUR",          /* Text name for the task. */
                    128,      /* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+2,/* Priority at which the task is created. */
                    NULL );      /* Used to pass out the created task's handle. */
										
	xTaskCreate(      command,       /* Function that implements the task. */
                    "COMMAND",          /* Text name for the task. */
                    128,      /* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                    NULL );      /* Used to pass out the created task's handle. */				

	xTaskCreate(      lecture_BP,       /* Function that implements the task. */
                    "LECT_BP",          /* Text name for the task. */
                    128,      /* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+3,/* Priority at which the task is created. */
                    NULL );      /* Used to pass out the created task's handle. */				
  /* USER CODE END 2 */

  

  /* Start scheduler */
	
   	vTaskStartScheduler();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */
/****************************************************************************/
/*       		 TACHE  MEMORISATION  BOUTON POUSSOIR PIETON			    */
/****************************************************************************/
void lecture_BP  (void *pvParameters) {  

	TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
	
  while ( 1 ) {
		
  	if ( HAL_GPIO_ReadPin( GPIOB, DPV1_Pin ))  DPV1 = 1;
																												// memorisation appui BP1 	
		if ( HAL_GPIO_ReadPin( GPIOB, DPV2_Pin ) )	DPV2 = 1;
						 					                           // Scrutation des touches toutes les 50ms
		if ( HAL_GPIO_ReadPin (GPIOC, SW1_Pin)) {	
		train = 1;
	  ph = 6;	  
		}
		if ( HAL_GPIO_ReadPin (GPIOC, SW2_Pin)) {
		train = 0;
		
		}
		vTaskDelayUntil( &xLastWakeTime, 50/ portTICK_PERIOD_MS);

  }
}

/****************************************************************************/
/*       				FONCTION 	SEQUENCEUR								*/
/****************************************************************************/
void sequenceur (bool valid_seq) { 
  
static char cpt;

	if(!modeManuel){
		if ( valid_seq ) {

			switch  ( ph ) {
			case 1: 
			{		
					HAL_GPIO_WritePin(GPIOB,R1_Pin|O1_Pin|R2_Pin, 1);
					HAL_GPIO_WritePin(GPIOB, V1_Pin|S1_Pin|V2_Pin|O2_Pin|S2_Pin, 0);
					cpt= 0;
					ph = 2;
					phtrain = 2;
					DPV1 = DPV2 = detect1 = detect2 = 0;	
			}
			break; 

			case 2:
			{
					 HAL_GPIO_WritePin(GPIOB,V1_Pin|S2_Pin|R2_Pin, 1);
					 HAL_GPIO_WritePin(GPIOB,R1_Pin|O1_Pin|V2_Pin|O2_Pin|S1_Pin, 0);
					if (( ++cpt > 8 ) || DPV1 || ( detect2&&!detect1&&!DPV2 )  ) {
					ph = 3;
					phtrain = 2;
					}
			}
			break;

			case 3:
			{		
				
					HAL_GPIO_WritePin(GPIOB, O1_Pin|R2_Pin, 1);
					HAL_GPIO_WritePin (GPIOB, R1_Pin|V1_Pin|S1_Pin|V2_Pin|O2_Pin|S2_Pin, 0);
					ph = 4;	
					phtrain = 3;
			}
			break;

			case 4:
			{		
					HAL_GPIO_WritePin (GPIOB, R1_Pin|R2_Pin|O2_Pin, 1);
					HAL_GPIO_WritePin (GPIOB, V1_Pin|O1_Pin|S1_Pin|V2_Pin|S2_Pin, 0);
					cpt = 0;
					ph = 5;	
					phtrain = 3;
					DPV1 = DPV2 = detect1 = detect2 = 0;
			}
			break;

			case 5:
			{
					HAL_GPIO_WritePin (GPIOB, R1_Pin|V2_Pin|S1_Pin, 1);
					HAL_GPIO_WritePin(GPIOB, V1_Pin|O1_Pin|S2_Pin|R2_Pin|O2_Pin, 0);	
					if (( ++cpt > 8 ) || DPV2 || ( detect1&&!detect2&&!DPV1 )) {
						ph = 6;
						phtrain = 3;
					}
			}
			break;

			case 6:
			{
					HAL_GPIO_WritePin (GPIOB, R1_Pin|O2_Pin, 1);
					HAL_GPIO_WritePin (GPIOB, V1_Pin|O1_Pin|S1_Pin|R2_Pin|V2_Pin|S2_Pin, 0);
					ph = 1;	
					phtrain = 1;
			}
			break;
			}
		}
		else {
			HAL_GPIO_WritePin (GPIOB, R1_Pin|V1_Pin|S1_Pin|R2_Pin|V2_Pin|S2_Pin, 0);
			HAL_GPIO_TogglePin(GPIOB,O1_Pin);
			HAL_GPIO_TogglePin(GPIOB, O2_Pin);
		}
	}
	else if (modeManuel){
		printf("test %d \n", ph);
		switch  ( ph ) {
		case 1:
		{
			printf("case 1");
			HAL_GPIO_WritePin(GPIOB,R1_Pin|O1_Pin|R2_Pin, 1);
			HAL_GPIO_WritePin(GPIOB, V1_Pin|S1_Pin|V2_Pin|O2_Pin|S2_Pin, 0);				
			if(nextStep) {
				ph = 2;	
				nextStep = 0;
			}
		}
		break;
		
		case 2:
		{
			printf("case 2");
			HAL_GPIO_WritePin(GPIOB,V1_Pin|S2_Pin|R2_Pin, 1);
			HAL_GPIO_WritePin(GPIOB,R1_Pin|O1_Pin|V2_Pin|O2_Pin|S1_Pin, 0);
			if(nextStep) {
				ph = 3;	
				nextStep = 0;
			}	
		}
		break;
		
		case 3:
		{
			printf("case 3");
			HAL_GPIO_WritePin(GPIOB, O1_Pin|R2_Pin, 1);
			HAL_GPIO_WritePin (GPIOB, R1_Pin|V1_Pin|S1_Pin|V2_Pin|O2_Pin|S2_Pin, 0);
			if(nextStep) {
				ph = 4;	
				nextStep = 0;
			}		
		}
		break;
		
		case 4:
		{
			printf("case 4");
			HAL_GPIO_WritePin (GPIOB, R1_Pin|R2_Pin|O2_Pin, 1);
			HAL_GPIO_WritePin (GPIOB, V1_Pin|O1_Pin|S1_Pin|V2_Pin|S2_Pin, 0);
			if(nextStep) {
				ph = 5;	
				nextStep = 0;
			}	
		}
		break;
		
		case 5:
		{
			printf("case 5");
			HAL_GPIO_WritePin (GPIOB, R1_Pin|V2_Pin|S1_Pin, 1);
			HAL_GPIO_WritePin(GPIOB, V1_Pin|O1_Pin|S2_Pin|R2_Pin|O2_Pin, 0);
			if(nextStep) {
				ph = 6;	
				nextStep = 0;
			}	
		}
		break;
		
		case 6:
		{
			printf("case 6");
			HAL_GPIO_WritePin (GPIOB, R1_Pin|O2_Pin, 1);
			HAL_GPIO_WritePin (GPIOB, V1_Pin|O1_Pin|S1_Pin|R2_Pin|V2_Pin|S2_Pin, 0);
			if(nextStep) {
				ph = 1;	
				nextStep = 0;
			}	
		}
		break;
		}
		
	}
	
}

void seqtrain (bool valid_seq)
{
	
	if(!modeManuel){
		if ( valid_seq ) {

			switch  ( phtrain ) {
			case 1: 
			{		
					HAL_GPIO_WritePin(GPIOB,R1_Pin|O1_Pin|R2_Pin, 1);
					HAL_GPIO_WritePin(GPIOB, V1_Pin|S1_Pin|V2_Pin|O2_Pin|S2_Pin, 0);
					phtrain = 2;
					ph = 2;
					DPV1 = DPV2 = detect1 = detect2 = 0;	
			}
			break; 

			case 2:
			{
					 HAL_GPIO_WritePin(GPIOB,V1_Pin|S2_Pin|R2_Pin, 1);
					 HAL_GPIO_WritePin(GPIOB,R1_Pin|O1_Pin|V2_Pin|O2_Pin|S1_Pin, 0);
					 phtrain = 2;
					 ph = 3;
			}
			break;


			case 3:
			{
					HAL_GPIO_WritePin (GPIOB, R1_Pin|O2_Pin, 1);
					HAL_GPIO_WritePin (GPIOB, V1_Pin|O1_Pin|S1_Pin|R2_Pin|V2_Pin|S2_Pin, 0);
					phtrain = 1;
					ph = 1;
			}
			break;
			}
		}
		else {
			HAL_GPIO_WritePin (GPIOB, R1_Pin|V1_Pin|S1_Pin|R2_Pin|V2_Pin|S2_Pin, 0);
			HAL_GPIO_TogglePin(GPIOB,O1_Pin);
			HAL_GPIO_TogglePin(GPIOB, O2_Pin);
		}
	}
}
/****************************************************************************/
/*               	FONCTION 	GENERATION TEMPS							*/
/****************************************************************************/
bool generation_temps ( void) { 

bool valid_seq;
    					                /* clock is an endless loop           */
    if (++horloge.sec == 60)  {         /* calculate the second               */
      horloge.sec = 0;
      if (++horloge.min == 60)  {       /* calculate the minute               */
        horloge.min = 0;
        if (++horloge.heure == 24)  {    /* calculate the hour                 */
          horloge.heure = 0;
        }
      }
    }
  
	aff_H.chaine = chaine1;
  aff_H.heure = horloge.heure;
	aff_H.min = horloge.min;
	aff_H.sec = horloge.sec;		
  if (!xQueueSend( xTxQueue, ( void * ) &aff_H, 100 / portTICK_PERIOD_MS ))  xQueueReset( xTxQueue );
	aff_H.chaine = chaine2;
  aff_H.heure = debut.heure;
	aff_H.min = debut.min;
	aff_H.sec = debut.sec;		
  if ( !xQueueSend( xTxQueue, ( void * ) &aff_H, 100 / portTICK_PERIOD_MS )) xQueueReset( xTxQueue );
	aff_H.chaine = chaine3;
  aff_H.heure = fin.heure;
	aff_H.min = fin.min;
	aff_H.sec = fin.sec;		
  if (!xQueueSend( xTxQueue, ( void * ) &aff_H, 100 / portTICK_PERIOD_MS )) xQueueReset( xTxQueue );

           
   
  	if (memcmp (&debut, &fin, sizeof ( struct temps)) < 0)  {
    	if (memcmp (&debut, &horloge, sizeof ( struct temps)) < 0  &&
        	memcmp (&horloge, &fin,   sizeof ( struct temps)) < 0)  valid_seq = 1;
		else valid_seq = 0;
  	}                                              
  	else  { 
    	if (memcmp (&fin,   &horloge, sizeof (debut)) > 0  &&
        	memcmp (&horloge, &debut, sizeof (debut)) > 0)  valid_seq = 1;
		else valid_seq = 0;
  	}

	return ( valid_seq );
} 

           
/****************************************************************************/
/*     FONCTION lecture commande et convertion en structure temps  			*/
/****************************************************************************/
bool lect_H (char  *buffer)  {
  signed char args;                          	     	/* number of arguments       */

  v_temps.sec = 0; 
	/* preset second             */
  args = sscanf (buffer, "%d:%d:%d",        		/* scan input line for       */
                 &v_temps.heure,                  	/* hour, minute and second   */
                 &v_temps.min,
                 &v_temps.sec);

  if (v_temps.heure > 23  ||  v_temps.min > 59  ||  /* check for valid inputs    */
      v_temps.sec > 59   ||  args < 2        ||  args == EOF)  {
	  
    printf ("\n*** ERROR: INVALID TIME FORMAT\n");
	 
    return (0);
  }
  return (1);
}

/****************************************************************************/
/*       				TACHE	CONTROLEUR									*/
/****************************************************************************/
void controleur  (void *pvParameters) {
  
	TickType_t xLastWakeTime;
	
	
 	ph = 1;
	phtrain = 3;
	// Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

 	while (1) {
		if (train == 0)
		{
		sequenceur(generation_temps());
		}
		else {
			seqtrain (generation_temps());
		}
		vTaskDelayUntil( &xLastWakeTime, 1000/ portTICK_PERIOD_MS);
 	}
}

/****************************************************************************/
/*       				TACHE	GESTION  DES COMMANDES						*/
/****************************************************************************/
void command  (void *pvParameters) {                  
  
	char cmde[16];						// en RAM interne pour accés rapide 
	char buff[50];
	char	c;
	char cnt,i = 0;
  struct print_H aff;
	modeManuel = 0;
	nextStep = 0;
	
		printf ( menu);
	
  	while (1)  { 
		 
		if 	( xQueueReceive( xTxQueue, &aff, 100 / portTICK_PERIOD_MS )) {
			
			printf( aff.chaine, aff.heure, aff.min, aff.sec );
			sprintf(buff, "%d: %d: %d", aff.heure, aff.min, aff.sec);
			printf ("\n %s", buff);
			lcd_clear();
		  lcd_print("chr :");
		  lcd_print(buff);
		}
			
		if ( xQueueReceive( xRxQueue, &c, 100 / portTICK_PERIOD_MS )) {
			
			if ( c == ESC ) {  		
			printf ( "\n\rCommandes :  ");	
			cnt= 0;	
			printf("Veuillez ecrire la commande et l'heure a changer au format XHH:MM:SS avec X = {H, D, F} \n");
			do {  
				xQueueReceive( xRxQueue, &c, portMAX_DELAY );
				cmde[cnt++] = c;
			} while ( c != CR );
				cmde[--cnt]= 0; // marquage fin de chaine
			
			task = 0xff; 	
		
			for ( i=0; cmde[i] !=0; i++ )	{		// convertion en Majuscule 
				cmde[i] = toupper(cmde[i]);
			}

			for ( i=0; cmde[i] == ' ';i++ );			// suppression des espaces 


			switch ( cmde [i] )	{

	 
				case 'H':   
																							// Set Time Command          
							if (lect_H (&cmde[i+1]))  {        			// read time input and       
									horloge.heure = v_temps.heure;            // store in 'ctime'          
									horloge.min  = v_temps.min;
									horloge.sec  = v_temps.sec;
					}
				break;

					case 'F':                                		// Set End Time Command     
							if ( lect_H (&cmde[i+1]))  {        		// read time input and       
									fin.heure = v_temps.heure;               // store in 'end'           
									fin.min  = v_temps.min;
									fin.sec  = v_temps.sec;
							 }
						break;

						case 'D':                                // Set Start Time Command    
							if ( lect_H (&cmde[i+1]))  {        // read time input and       
									debut.heure = v_temps.heure;             // store in 'start'       
									debut.min   = v_temps.min;
									debut.sec   = v_temps.sec;
						}
						break;
				}   
			}
			if( c == 0x4D || c == 0x6D) { //Si on appuie sur la touche M ou m on passe en mode manuel
				if(!modeManuel){
					modeManuel = 1;
					ph = 1;
					printf("Bonjour, bienvenue dans le mode manuel ! \n");
				}
			}
			if ( c == 0x51 || c == 0x71) { //Si on appuie sur la touche Q ou q on sort en mode manuel
				if(modeManuel){
					modeManuel = 0;
					printf("Vous sortez du mode manuel, au revoir : \n");
				}
			}
			if ((c == 0x41 || c == 0x61) && modeManuel){ //Si on appuie sur la touche A ou a on sort en mode manuel
				nextStep = 1;
				printf("Next step : %d \n", (ph%6)+1);
			}
			
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
/* USER CODE BEGIN Callback 1 */

/* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
