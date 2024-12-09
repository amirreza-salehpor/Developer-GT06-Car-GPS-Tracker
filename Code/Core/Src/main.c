/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
	* @author   			: Amirreza Salehpour
	* @version        : Ver 1.0.1
	* @date           : 22 July 2023
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//--------------------------- ADC Variables
uint16_t adcBuffer[1];            //Buffer for store the results of the ADC conversion.
bool Enable_Triger=0;							//It's a kind of flag and when we have Triggered from inputs, this bit will become 1.*******
int Check_Timer1=0;								//It's kind of Counter for make time by using Timer 1 (by using this varriable, we make 1 Second).
int Number_Repeat=0;							//We control the number of responses when we have triggered from inputs.

//--------------------------- UART Variables
uint8_t byte;
char buffer[16];                  //It's for printf Function in UART.
int conter1=0;                    //It's for counting bytes received from UART.
char save_byte[50];               //It's for Storge received bytes.
bool block_Response=0;						//It's kind of flag for give a permission to print "TRIGGER" when we have Trig from Inputs.
bool block_UART=0;								//It's a kind of flag that use for enabling the response loop when we have "Command" from UART's input.
int sum_characters=0;							//It using for make partition for Commands.			
//int * arr_dynamic = NULL;
//int elements = 15;
//--------------------------- Variables for make Frame
int frame_counter=0;							//It's for counting bytes received from UART that they are valid for making Frame. 
int arr_frame[15];								//This array use for Storge valid bytes that sent from master device.
bool block_Counter_frame=0;				//It's for checking start of frame "0xAA";
int clear_counter_Frame=0;        //It using for clear "arr_frame";

//----------------Variable for make suitable outputs
bool Mode_Pulse=0;								//It's for selecting "pulse mode".
bool Mode_N_Counter=0;						//It's for selecting "N-Counter mode".

bool Select_Flag_1=0;							//We are able to select O- in our outpus by using this bit
bool Select_Flag_2=0;							//We are able to select O+ in our outpus by using this bit
bool Select_Flag_3=0;							//We are able to select O+ and O- in our outpus at the same time by using this bit
bool Select_Flag_4=0;							//we use this bit when we gonna float our outputs.

long int High_Time=0;             //It using for calculating high time in our outputs (It's just for positive output).
long int Low_Time=0;							//It using for calculating Low time in our outputs (It's just for positive output).
long int Number_Counter=0;				//It using for counting, IF we are in "N-counter mode"(It's just for positive output).

long int High_Time_2=0;           //It using for calculating high time in our outputs (It's just for negative output).
long int Low_Time_2=0;						//It using for calculating Low time in our outputs (It's just for negative output).
long int Number_Counter_2=0;			//It using for counting, IF we are in "N-counter mode"(It's just for negative output).



long int Counter_Pulse=0;					//It's a kind of counter that using for switching between high time and low time.(It's just for positive output).
long int Counter_Pulse_2=0;				//It's a kind of counter that using for switching between high time and low time.(It's just for negative output).
long int Counter_N_Counter=0;			//It's a kind of counter that using for Count down if we are in "N-counter mode".(It's just for positive output).
long int Counter_N_Counter_2=0;		//It's a kind of counter that using for Count down if we are in "N-counter mode".(It's just for negative output).

//bool Save_Status=0;
bool Save_Status_Trigger=0;       //If we have Trige in input this variable save statuse Enable_Triger
bool lock_repeat_Trigger=0;				//This Variable prevent from repeat print "TRIGGER" 
bool pulse_mode_positive_output=0;//It's for select positive output at same time by negative output
bool pulse_mode_negative_output=0;//It's for select negative output at same time by positive output





/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//------------------------------------------------------------------------------------------ ADC DMA
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1){// the Continuos Conversion Mode
	
	//Read_ACC();
	HAL_ADC_Start_IT(hadc1); // Re-Start ADC1 under Interrupt
                         
}
//------------------------------------------------------------------------------------------ TIMER 100 ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{  
	if (htim==&htim1) //10Hz //100ms
  {
		//--------------------------------------------------------- 
		
		if(Enable_Triger==1 && Save_Status_Trigger==0 && lock_repeat_Trigger==0){
			Save_Status_Trigger=1;
			block_Response=1;
			Number_Repeat=0;
			Check_Timer1=0;
		}
		if(Enable_Triger==0){
			//------------------------------- If you remove this loop we will sense fall and down Trige together
			if(lock_repeat_Trigger==1){
				 Save_Status_Trigger=0;
			}
			//---------------------------------
			lock_repeat_Trigger=0;
		}	
		
		if(Check_Timer1%10==0 && Save_Status_Trigger==1 && lock_repeat_Trigger==0){ // every 1 Second we will have response 
			Check_Timer1=0;
			block_Response=1;
			Number_Repeat=Number_Repeat+1;	
			if(Number_Repeat>3){ // Repeat response for 3 times
				lock_repeat_Trigger=1;
				Save_Status_Trigger=0;
				Check_Timer1=0;
				Number_Repeat=0;
				block_Response=0;
			}
		}
		if(Save_Status_Trigger==1){
			Check_Timer1=Check_Timer1+1;
		}
		//---------------------------------------------------------
		
		//-------------------------------------------------------------------------------------------------PULSE Mode Timre
		if(Mode_Pulse==1){
			//------------------------------------------------------------------- Positive Output
			if(High_Time>0 && pulse_mode_positive_output==1){
				if(Counter_Pulse<High_Time){ //High Time
					if(High_Time>0 && Select_Flag_3==0){ //O+ is high
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
					}	
					else if(Select_Flag_3==1){//O+ and O+ is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
					}		
				}
				else if(Counter_Pulse>=High_Time && Counter_Pulse<High_Time+Low_Time){ //Low Time
					
					if(High_Time>0 && Select_Flag_3==0){ //O+ is high
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
					}	
					else if(Select_Flag_3==1){//O+ and O+ is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
					}		
				}
				Counter_Pulse=Counter_Pulse+1;
				if(Counter_Pulse>=High_Time+Low_Time){
					Counter_Pulse=0;
				}
			}
			//-------------------------------------------------------------------
			//------------------------------------------------------------------- Negative Output
			if(High_Time_2>0 && pulse_mode_negative_output==1){
				if(Counter_Pulse_2<High_Time_2){ //High Time
					
					if(High_Time_2 && Select_Flag_3==0){ // O- is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
					}
				}
				else if(Counter_Pulse_2>=High_Time_2 && Counter_Pulse_2<High_Time_2+Low_Time_2){ //Low Time
					if(High_Time_2 && Select_Flag_3==0){ // O- is LOW
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
					}	
				}
				Counter_Pulse_2=Counter_Pulse_2+1;
				if(Counter_Pulse_2>=High_Time_2+Low_Time_2){
					Counter_Pulse_2=0;
				}
			}
			//-------------------------------------------------------------------
		}
		
		
		
		
		//-------------------------------------------------------------------------------------------------N-Counter Mode Timre
		
		if(Mode_N_Counter==1 && (Number_Counter || Number_Counter_2)>0){	
			//------------------------------------------------------------------- Positive Output	
			if(High_Time>0 && pulse_mode_positive_output==0){
				if(Counter_Pulse<High_Time){ //High Time
					if(High_Time>0 && Select_Flag_3==0){ //O+ is high
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
					}	
					else if(Select_Flag_3==1){//O+ and O+ is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
					}		
				}
				else if(Counter_Pulse>=High_Time && Counter_Pulse<High_Time+Low_Time){ //Low Time
					if(High_Time>0 && Select_Flag_3==0){ //O+ is high
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
					}	
					else if(Select_Flag_3==1){//O+ and O+ is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
					}		
				}
				Counter_Pulse=Counter_Pulse+1;
				if(Counter_Pulse>=High_Time+Low_Time){
					Counter_Pulse=0;
					Counter_N_Counter=Counter_N_Counter+1;
				}
				if(Counter_N_Counter>(Number_Counter-1)){
					Counter_N_Counter=0;
					//Mode_N_Counter=0;
					High_Time=0;
					Low_Time=0;
					Number_Counter=0;
				}
			}
			//-------------------------------------------------------------------
			//------------------------------------------------------------------- Negative Output			
			if(High_Time_2>0 && pulse_mode_negative_output==0){
				if(Counter_Pulse_2<High_Time_2){ //High Time
					if(High_Time_2 && Select_Flag_3==0){ // O- is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
					}	
				}
				else if(Counter_Pulse_2>=High_Time_2 && Counter_Pulse_2<High_Time_2+Low_Time_2){ //Low Time
					if(High_Time_2 && Select_Flag_3==0){ // O- is high
						HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
					}
				}
				Counter_Pulse_2=Counter_Pulse_2+1;
				if(Counter_Pulse_2>=High_Time_2+Low_Time_2){
					Counter_Pulse_2=0;
					Counter_N_Counter_2=Counter_N_Counter_2+1;
				}
				if(Counter_N_Counter_2>(Number_Counter_2-1)){
					Counter_N_Counter_2=0;
					//Mode_N_Counter=0;
					High_Time_2=0;
					Low_Time_2=0;
					Number_Counter_2=0;
				}
				//-------------------------------------------------------------------
			}
			
			if(High_Time_2==0 && High_Time==0){
				Mode_N_Counter=0;
			}
		}
		//HAL_GPIO_TogglePin(OUT2_GPIO_Port, OUT2_Pin);
	}
}


//------------------------------------------------------------------------------------------ UART 2
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2) {
		
		//------------------------------------------- Make frame
		if(byte==0xAA){
			block_Counter_frame=1;
			frame_counter=0;
			for(clear_counter_Frame=0;clear_counter_Frame<15;clear_counter_Frame++){
				arr_frame[clear_counter_Frame]=0;
			}
			//free(arr_frame);
			//arr_dynamic = calloc(elements, sizeof(int)); //Array with 2 integer blocks
		}
		if(block_Counter_frame==1 && frame_counter<15){
			arr_frame[frame_counter]=byte;
			frame_counter=frame_counter+1;
		}
	
		//------------------------------------------- Receive Commands
		if(byte>=97 && byte<=122){
			byte=byte-32;
		}
		if(byte!=13 && byte!=10 && block_Counter_frame==0){
			//arr_dynamic[conter1]=byte;
			sum_characters=sum_characters+(byte*conter1);
			conter1++;	
		}
		else if(byte==13 && block_Counter_frame==0){
			block_UART=1;
			conter1=0;
		}
		HAL_UART_Receive_IT(&huart2, &byte, 1);
	}
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adcBuffer[0],1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	HAL_UART_Receive_IT(&huart2, &byte, 1);	
	//arr_dynamic = calloc(elements, sizeof(int)); //Array with 2 integer blocks
	
  while (1)
  {
    /* USER CODE END WHILE */
		
		Enable_Triger=!(HAL_GPIO_ReadPin(DP_INPUT_GPIO_Port, DP_INPUT_Pin));
		
		
				
		if(block_UART==1){
			//---------------------------------------------------- Commands 
			if(sum_characters==1221){//STATUS 
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "STATUS=%d,%d\r\n",adcBuffer[0],Enable_Triger), 500);
				lock_repeat_Trigger=1;
				block_Response=0;
				Save_Status_Trigger=0;
//				Save_Status_Trigger=0;
//				Check_Timer1=0;
//				Number_Repeat=0;
//				block_Response=0;
			}
			//----------------------------------------------------
			else if(sum_characters==1536){//POSOUT1
				Mode_Pulse=0;
				Mode_N_Counter=0;
				HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "OK\r\n"), 500);
			}
			else if(sum_characters==1530){//POSOUT0
				Mode_Pulse=0;
				Mode_N_Counter=0;
				HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "OK\r\n"), 500);
			}
			//----------------------------------------------------
			else if(sum_characters==1502){//NEGOUT1
				Mode_Pulse=0;
				Mode_N_Counter=0;
				HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "OK\r\n"), 500);
			}
			else if(sum_characters==1496){//NEGOUT0
				Mode_Pulse=0;
				Mode_N_Counter=0;
				HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "OK\r\n"), 500);
			}
			//----------------------------------------------------
			else if(sum_characters==5429){//RESETOUTPUTS
				Mode_Pulse=0;
				Mode_N_Counter=0;
				HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "OK\r\n"), 500);
			}
			//----------------------------------------------------
			else if(sum_characters==75){//OK
				Save_Status_Trigger=0;
				Check_Timer1=0;
				Number_Repeat=0;
				lock_repeat_Trigger=1;
				block_Response=0;
			}
			sum_characters=0;
			block_UART=0;
		}
		
		
		//---------------------------------------------------- Input Triger
		if(block_Response==1 && lock_repeat_Trigger==0){
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer, "TRIGGER\r\n"), 500);
			block_Response=0;
		}
		//---------------------------------------------------- Check Frame --------------------
		if(arr_frame[0]==0xaa && arr_frame[1]==0xbb && arr_frame[13]==0xee && arr_frame[14]==0xcc && (frame_counter-1)==14){
			block_Counter_frame=0;
//			for(i=0;i<frame_counter;i++){
//				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer," %d",arr_dynamic[i]), 500);
//			}
			//a=arr_frame[2]&&0x10;
			//------------------------------------------------------------- Select OUT
			if((arr_frame[2]&0x0C)==0x0C){// O+:Select O-:Select 
				Select_Flag_1=0;
				Select_Flag_2=0;
				Select_Flag_3=1;
//				Select_Flag_4=0;
			}
			else if((arr_frame[2]&0x0C)==0x00){// O+:Float O-:Float 
				Select_Flag_1=0;
				Select_Flag_2=0;
				Select_Flag_3=0;
				Select_Flag_4=1;
				HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
			}
			else if((arr_frame[2]&0x08)==0x08){// O+:Select O-:Float 
				Select_Flag_1=0;
				Select_Flag_2=1;
				Select_Flag_3=0;
				Select_Flag_4=0;
			}
			else if((arr_frame[2]&0x04)==0x04){ // O+:Float O-:Select 
				Select_Flag_1=1;
				Select_Flag_2=0;
				Select_Flag_3=0;
				Select_Flag_4=0;
			}
			
			
			//--------------------------------------------------------------------------------------- Mods
			
			//------------------------------------- N counter 
			if(Select_Flag_4==0 && (arr_frame[2]&0x03)==0x03){
				
				if(Select_Flag_2==1){ // Positive Output
					pulse_mode_positive_output=0;
					Counter_N_Counter=0;
					Counter_Pulse=0;
					High_Time=(arr_frame[5]*65536)+((arr_frame[6]*256)+arr_frame[7]);
					Low_Time=(arr_frame[8]*65536)+((arr_frame[9]*256)+arr_frame[10]);
					Number_Counter=(arr_frame[11]*256)+arr_frame[12];
				}
				
				if(Select_Flag_1==1){ // Negative Output
					pulse_mode_negative_output=0;
					Counter_Pulse_2=0;
					Counter_N_Counter_2=0;
					High_Time_2=(arr_frame[5]*65536)+((arr_frame[6]*256)+arr_frame[7]);
					Low_Time_2=(arr_frame[8]*65536)+((arr_frame[9]*256)+arr_frame[10]);
					Number_Counter_2=(arr_frame[11]*256)+arr_frame[12];
				}
				
				if(Select_Flag_3==1){
					pulse_mode_positive_output=0;
					pulse_mode_negative_output=0;
					Counter_N_Counter=0;
					Counter_Pulse=0;
					Counter_Pulse_2=0;
					Counter_N_Counter_2=0;
					High_Time=(arr_frame[5]*65536)+((arr_frame[6]*256)+arr_frame[7]);
					Low_Time=(arr_frame[8]*65536)+((arr_frame[9]*256)+arr_frame[10]);
					Number_Counter=(arr_frame[11]*256)+arr_frame[12];
					High_Time_2=High_Time;
					Low_Time_2=Low_Time;
					Number_Counter=Number_Counter_2;
				}
				
				if(Number_Counter!=0x00 && Number_Counter_2!=0x00 && Select_Flag_3==1){
					Mode_N_Counter=1;
				}
				
				else if(Number_Counter!=0x00 && Select_Flag_2==1 ){
					Mode_N_Counter=1;
				}
				
				else if(Number_Counter_2!=0x00 && Select_Flag_1==1 ){
					Mode_N_Counter=1;
				}

			}
			
			//------------------------------------- Float 
			else if(Select_Flag_4==0 && (arr_frame[2]&0x03)==0x00){
				
				if(Select_Flag_1==1){ // O- is high
					HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
					Select_Flag_1=0;
					High_Time_2=0;
					Low_Time_2=0;
					Number_Counter_2=0;
				}
				else if(Select_Flag_2==1){ //O+ is high
					HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
					Select_Flag_2=0;
					High_Time=0;
					Low_Time=0;
					Number_Counter=0;
				}	
				else if(Select_Flag_3==1){//O- and O+ is high
					HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
					High_Time=0;
					Low_Time=0;
					Number_Counter=0;
					Select_Flag_3=0;
					High_Time_2=0;
					Low_Time_2=0;
					Number_Counter_2=0;
				}	
			}
			
			//------------------------------------- Pulse 
			else if(Select_Flag_4==0 && (arr_frame[2]&0x02)==0x02){
				
				if(Select_Flag_2==1){ // Positive Output
					Counter_Pulse=0;
					pulse_mode_positive_output=1;
					High_Time=(arr_frame[5]*65536)+((arr_frame[6]*256)+arr_frame[7]);
					Low_Time=(arr_frame[8]*65536)+((arr_frame[9]*256)+arr_frame[10]);
					Number_Counter=(arr_frame[11]*256)+arr_frame[12];
				}
				if(Select_Flag_1==1){ // Negative Output
					pulse_mode_negative_output=1;
					High_Time_2=(arr_frame[5]*65536)+((arr_frame[6]*256)+arr_frame[7]);
					Low_Time_2=(arr_frame[8]*65536)+((arr_frame[9]*256)+arr_frame[10]);
					Number_Counter_2=(arr_frame[11]*256)+arr_frame[12];
					Counter_Pulse_2=0;
				}
				if(Select_Flag_3==1){
					pulse_mode_positive_output=1;
					pulse_mode_negative_output=1;
					High_Time=(arr_frame[5]*65536)+((arr_frame[6]*256)+arr_frame[7]);
					Low_Time=(arr_frame[8]*65536)+((arr_frame[9]*256)+arr_frame[10]);
					Number_Counter=(arr_frame[11]*256)+arr_frame[12];
					High_Time_2=High_Time;
					Low_Time_2=Low_Time;
					Number_Counter_2=Number_Counter;
					Counter_Pulse=0;
					Counter_Pulse_2=0;
				}
				
				if(Number_Counter==0x00 && Number_Counter_2!=0x00 && Select_Flag_3==1){
					Mode_Pulse=1;
				}
				else if(Number_Counter==0x00 && Select_Flag_2==1 ){
					Mode_Pulse=1;
				}
				else if(Number_Counter_2==0x00 && Select_Flag_1==1 ){
					Mode_Pulse=1;
				}
			}
			//------------------------------------- Active 
			else if(Select_Flag_4==0 && (arr_frame[2]&0x01)==0x01 ){ 
				
				if(Select_Flag_1==1){ // O- is high
					HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
					Select_Flag_1=0;
					High_Time_2=0;
					Low_Time_2=0;
					Number_Counter_2=0;
				}
				else if(Select_Flag_2==1){ //O+ is high
					HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
					Select_Flag_2=0;
					High_Time=0;
					Low_Time=0;
					Number_Counter=0;
				}	
				else if(Select_Flag_3==1){//O- and O+ is high
					HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
					High_Time=0;
					Low_Time=0;
					Number_Counter=0;
					Select_Flag_3=0;
					High_Time_2=0;
					Low_Time_2=0;
					Number_Counter_2=0;
				}	
			}
			
			
			
			
			//---------------------------
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer,"OK\r\n"), 500);
			frame_counter=0;
			//free(arr_frame);
			for(clear_counter_Frame=0;clear_counter_Frame<15;clear_counter_Frame++){
				arr_frame[clear_counter_Frame]=0;
			}
		}
		//----------------------------------------------------------------------------------------
		//		else if(frame_counter>=15){
//			free(arr_dynamic);
//			block_Counter_frame=0;
//			frame_counter=0;
//		}
		
//		for(i=0;i<=frame_counter;i++){
//			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, sprintf(buffer,"\r\n%d",arr_dynamic[i]), 500);
//		}
//		HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_7CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1599;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUT2_Pin|OUT1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : DP_INPUT_Pin */
  GPIO_InitStruct.Pin = DP_INPUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DP_INPUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT2_Pin OUT1_Pin */
  GPIO_InitStruct.Pin = OUT2_Pin|OUT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
