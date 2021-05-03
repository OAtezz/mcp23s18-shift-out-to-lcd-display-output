//..................Project Asahi..........................//
//..................Board : Sumitomo Modify................//
//..................Firmware : FW_104......................//
//..................DYGISTECH Co., Ltd.....................//
//..................Date : 5/4/2021........................//

// demo SW_OK, LCD_OK, LED change pin A1,2,3 => D7,11,12 //

#include <TimerOne.h>
#include <EEPROM.h>
//#include <LiquidCrystal.h>

#include <SPI.h>
#include <MCP23S17_LCD.h>

MCP23S17_LCD lcd(PC6, A5, PORTB); // init chip mcp23s18

//LiquidCrystal lcd(A5, A4, A0, A1, A2, A3); // RS, E, D4, D5, D6, D7

#define FIRMWARE_VS 106

//---------- Define For 74xx595 ----------//
//#define LATCHPIN_595  A3
//#define CLOCKPIN_595  A4
//#define DATAPIN_595   A2
//#define OUTPUTEN_595  A5
//----------------------------------------//

//---------- Define For 74xx165 ----------//
#define DATA_WIDTH        8
#define PULSE_WIDTH_USEC  5
#define POLL_DELAY_MSEC   1
#define PLOADPIN_165      10
#define DATAPIN_165       8
#define CLOCKPIN_165      9
//----------------------------------------//

//--------- Define In/Out Switch ---------//
//Input
#define SW_F1 6
#define SW_F2 5
#define SW_F1_F2 (SW_F1+SW_F2)
#define SW_G  4
#define SW_O  3
#define SW_R  2
//Output
//#define Buzzer 7
//#define Out4 A0
//#define Out5 A1
//----------------------------------------//

#define RS485_DIR 13
#define RS485_RX LOW
#define RS485_TX HIGH

byte Unit_Status;

//EEPROM ADDRESS
const byte Add_PickQty[3] = {0,2,4};    //unsigned int
const byte Add_PickReg[3] = {6,7,8};    //byte 
const byte Add_SAVENSEC_Num = 10;       //unsigned int
const byte Add_SAVENSEC_Status = 12;    //byte
const byte Add_LED_Status[8] = {13,14,15,16,17,18,19,20};//byte

const byte Add_RED = 24;
const byte Add_GREEN = 25;
const byte Add_BLUE = 26;

const byte Add_LED9_Status = 21;
const byte Add_C_control = 22;
const byte Add_D_control = 23;


const byte CharDecode[18] = {
  0x3F, // 0)0011 1111 [0] 0x3F
  0x06, // 1)0000 0110 [1] 0x06
  0x5B, // 2)0101 1011 [2] 0x5B
  0x4F, // 3)0100 1111 [3] 0x4F
  0x66, // 4)0110 0110 [4] 0x66
  0x6D, // 5)0110 1101 [5] 0x6D
  0x7D, // 6)0111 1101 [6] 0x7D
  0x07, // 7)0000 0111 [7] 0x07
  0x7F, // 8)0111 1111 [8] 0x7F
  0x6F, // 9)0110 1111 [9] 0x6F
  0x77, //10)0111 0111 [A] 0x77
  0x7C, //11)0111 1100 [b] 0x7C
  0x39, //12)0011 1001 [C] 0x39
  0x5E, //13)0101 1110 [d] 0x5E
  0x79, //14)0111 1001 [E] 0x79
  0x71, //15)0111 0001 [F] 0x71
  0x00, //16)0000 0000 [ ] 0x00
  0x80};//17)1000 0000 [.] 0x10
  
const byte Digit[5] = {
  0x08, //0000 0001
  0x04, //0000 0010
  0x02, //0000 0100
  0x01, //0000 1000
  0x00//0001 0000
  }; 

 

byte digit_count = 0;
byte DigitWrite[7];

#define X100MS 40 //2.5*40 = 100mS
unsigned int bling_count1 = 0;//
bool fbling_count1 = 0;

#define X1000MS 200 //2.5*400 = 1000mS
unsigned int bling_count2 = 0;//
bool fbling_count2 = 0;

//Switch Delay
#define X50MS 20 //2.5*20 = 50mS
#define X150MS 60 //2.5*60 = 150mS

unsigned int bling_count3 = 0;//
bool fbling_count3 = 0;

//0 off, 1 on, 2 bling
#define S_OFF           0
#define S_ON            1
#define S_BLING         2
#define S_PRINT_ADDRESS 3
#define S_PRINT_ERR     4
#define S_PRINT_FW      5

byte LED_Status[8] = {0,0,0,0,0,0,0,0};
byte LED9_Status = 0;
byte C_control = 0;
byte D_control = 0;

byte RED = 0;
byte GREEN = 0;
byte BLUE = 0;


byte SAVENSEC_Status = 0;
unsigned int SAVENSEC_Num = 0;

#define U_TOTAL   8 

#define U_LED1   0 
#define U_LED2   1
#define U_LED3   2
#define U_LED4   3
#define U_LED5   4
#define U_LED6   5
#define U_LED7   6
#define U_LED8   7

//#define LED9 0

#define R_FINISH    3 
#define R_INPROCESS 2 
#define R_WAITING   1 
#define R_FREE      0
  
unsigned int PickQty[3]={0,0,0};
byte PickReg[3]={0,0,0};

byte fSW_F1 = 0;
byte fSW_F2 = 0;
byte fSW_F1_F2 = 0;
byte fSW_G = 0;
byte fSW_O = 0;
byte fSW_R = 0;
unsigned int countSW_F1 = 0;
unsigned int countSW_F2 = 0;
unsigned int countSW_F1_F2 = 0;
unsigned int countSW_G = 0;
unsigned int countSW_O = 0;
unsigned int countSW_R = 0;

byte fDisableInputSwitch = false;

byte Unit_Address = 0;
byte Hub_Address = 0;
byte Count_IncomingByte = 0;
byte inByte = 0;
int IncomingByte[20];
bool fByteComplete = false;  // whether the string is complete

#define ENQ 0x05
#define EOT 0x04
#define STX 0x02
#define ACK 0x06
#define CRT 0x0D

//RTV code  
#define RTV_OK  0xA0//Return value -> OK
#define RTV_REG_ERR 0xF0//Return value -> Register Error
#define RTV_CHS_ERR 0xF1//Return value -> Check Sum Error
#define RTV_BUS_ERR 0xF2//Return value -> Busy
#define RTV_CMD_ERR 0xF3//Return value -> Command

#define RTV_UST 0xB0//Return User Status

const String CMD_WriteOutputREG = String("WOR");
const String CMD_EnableDisableInput = String("EDI");
const String CMD_ReadInputREG = String("RIR");
const String CMD_ClearInputREG = String("CIR");
const String CMD_PrintAddress = String("PAD");
const String CMD_WriteBlingOutputREG = String("WOR");
const String CMD_WriteLCD = String("LCD"); //

byte fPrintAddress = false;

String CMD_Input = String("NNN");

byte fStartComm = 0;
const float a = 0.001;

byte InputCount[5] = {0,0,0,0,0};
byte InputEnable[5] = {0,0,0,0,0};

//LCD define for chip MCP23S18
#define PORTA  0x14
#define PORTB  0x15
#define IODIRA 0x00 //BANK[0]
#define IODIRB 0x01 //
#define IOCONA 0x0A //
#define IOCONB 0x0B // 

#define GPIOA 0x12 // 0x09
// Control byte and configuration register information
// Control Byte: "0100 A2 A1 A0 R/W" -- W=0
#define    OPCODEW       (0b01000000)  
// Opcode for MCP23S17 with LSB (bit0) set to write (0), 
// address OR'd in later, bits 1-3
#define    OPCODER       (0b01000001)  
// Opcode for MCP23S17 with LSB (bit0) set to read (1), 
// address OR'd in later, bits 1-3

void setup() {
   
  Serial.begin(9600);
  //Serial.begin(38400);
  //Serial.begin(115200);

//----------- Define For MCP23S18 ----------//  
  lcd.begin(20, 4);
  
  lcd.gpioPinMode(OUTPUT);
  lcd.gpioPort(0x0000);
  
  lcd.setCursor(0, 0); // 
  lcd.print("XXXXXXXXXXXXXXXXXXXX");
  delay(1000);
  
  lcd.clear();
  delay(1000);
 
//---------- Define For RS485_DIR ----------//
  pinMode(RS485_DIR, OUTPUT);
//------------------------------------------//

//---------- Define For 74xx595 ----------//
  //pinMode(LATCHPIN_595, OUTPUT);
  //pinMode(CLOCKPIN_595, OUTPUT);
  //pinMode(DATAPIN_595, OUTPUT);
  //pinMode(OUTPUTEN_595, OUTPUT);
//----------------------------------------//
   
//---------- Define For 74xx165 ----------//
  pinMode(PLOADPIN_165, OUTPUT);
  pinMode(CLOCKPIN_165, OUTPUT);
  pinMode(DATAPIN_165, INPUT);
  digitalWrite(CLOCKPIN_165, LOW);
  digitalWrite(PLOADPIN_165, HIGH);
//----------------------------------------//

//--------- Define In/Out Switch ---------//
  pinMode(SW_F1, INPUT);
  pinMode(SW_F2, INPUT);
  pinMode(SW_G, INPUT);
  pinMode(SW_O, INPUT);
  pinMode(SW_R, INPUT);
  //pinMode(Buzzer, OUTPUT);
  //digitalWrite(Out4, 0);
  //digitalWrite(Out5, 0);
  //pinMode(Out4, OUTPUT);
  //pinMode(Out5, OUTPUT);
  
  pinMode(U_LED1, OUTPUT);
  pinMode(U_LED2, OUTPUT);
  pinMode(U_LED3, OUTPUT);
  pinMode(U_LED4, OUTPUT);
  pinMode(U_LED5, OUTPUT);
  pinMode(U_LED6, OUTPUT);
  pinMode(U_LED7, OUTPUT);

  lcd.gpioDigitalWrite(U_LED1, 0);
  lcd.gpioDigitalWrite(U_LED2, 0);
  lcd.gpioDigitalWrite(U_LED3, 0);
  lcd.gpioDigitalWrite(U_LED4, 0);
  lcd.gpioDigitalWrite(U_LED5, 0);
  lcd.gpioDigitalWrite(U_LED6, 0);
  lcd.gpioDigitalWrite(U_LED7, 0);
//----------------------------------------//
  //analogWrite(U_LED1, 0);
  //analogWrite(U_LED2, 0);
  //analogWrite(U_LED3, 0);
  
  //digitalWrite(Out4, 0);
  //digitalWrite(Out5, 0);

//-------------- End LCD ----------------//

  Unit_Address = read_module_address();
  Timer1.initialize(2500);//2.5mS
  Timer1.attachInterrupt( timer1Isr );
  
  fDisableInputSwitch = false;
  fPrintAddress = false;
  getEEPROM_Data();
  Update_4x7SEC();
 
}

 void loop(){
  //new test funtion
/*
        for (int i=0;i<8;i++){
    lcd.gpioDigitalWrite(i,1);
    delay(300);
    }
    
    for (int i=0;i<8;i++){
    lcd.gpioDigitalWrite(i,0);
    delay(300);
    }
    
  lcd.setCursor(0, 1);
  lcd.print("Hi,DYGISTECH_Co.,Ltd");
  delay(500);

  lcd.clear();
  delay(500);
  */
  //Switch Function F1 => Print Address
  if(fSW_F1){
    if(InputEnable[0] == 1){
      InputCount[0] = InputCount[0]+1;
    }
    //---fSW_F1--------------------------------------------------------
    if(fPrintAddress == false){
      fDisableInputSwitch = true;
      fPrintAddress = true;
      SAVENSEC_Status = S_PRINT_ADDRESS;
      SAVENSEC_Num  = Unit_Address;
      D_control = S_OFF;
      C_control = S_OFF;
      LED9_Status = S_OFF;
      LED_Status[U_LED8] = S_OFF;
      LED_Status[U_LED7] = S_OFF;
      LED_Status[U_LED6] = S_OFF;
      LED_Status[U_LED5] = S_OFF;
      LED_Status[U_LED4] = S_OFF;
      LED_Status[U_LED3] = S_OFF;
      LED_Status[U_LED2] = S_OFF;
      LED_Status[U_LED1] = S_OFF;
      Update_4x7SEC();
      
    }
    else{
      fDisableInputSwitch = false;
      fPrintAddress = false;
      getEEPROM_Data();
      Update_4x7SEC();
    }

    //---fSW_F1--------------------------------------------------------
    fSW_F1 = 0;
  }
  
  //Switch Function F2 > Exit Process
  if(fSW_F2){
    if(InputEnable[1] == 1){
      InputCount[1] = InputCount[1]+1;
    }
    //---fSW_F2--------------------------------------------------------
    if(PickReg[U_LED1]==R_INPROCESS){
      PickReg[U_LED1] = R_WAITING;
      LED_Status[U_LED1] = S_ON;
    }
    if(PickReg[U_LED2]==R_INPROCESS){
      PickReg[U_LED2] = R_WAITING;
      LED_Status[U_LED2] = S_ON;
    }
    if(PickReg[U_LED3]==R_INPROCESS){
      PickReg[U_LED3] = R_WAITING;
      LED_Status[U_LED3] = S_ON;
    }

    SAVENSEC_Num = 0;
    SAVENSEC_Status = S_BLING;
      
    putEEPROM_Data();
    Update_4x7SEC();
    //Beep_Enter();

    //---fSW_F2--------------------------------------------------------
    fSW_F2 = 0;
  }

  //Switch Function G
  if(fSW_G){
    if(InputEnable[2] == 1){
      InputCount[2] = InputCount[2]+1;
    }
    
    //---fSW_G--------------------------------------------------------
    if(PickReg[U_LED1]==R_WAITING){
      
      if(PickReg[U_LED2]==R_INPROCESS){
        PickReg[U_LED2] = R_WAITING;
        LED_Status[U_LED2] = S_ON;
      }
      if(PickReg[U_LED3]==R_INPROCESS){
        PickReg[U_LED3] = R_WAITING;
        LED_Status[U_LED3] = S_ON;
      }
      PickReg[U_LED1] = R_INPROCESS;
      
      SAVENSEC_Num = PickQty[U_LED1];
      SAVENSEC_Status = S_ON;
      LED_Status[U_LED1] = S_BLING;
      
      putEEPROM_Data();
      Update_4x7SEC();
      //Beep_Enter();
      
    }else if((PickReg[U_LED1]==R_INPROCESS)){
      PickReg[U_LED1] = R_FINISH;
      SAVENSEC_Num = PickQty[U_LED1];
      SAVENSEC_Status = S_BLING;
      LED_Status[U_LED1] = S_OFF;
      
      putEEPROM_Data();
      Update_4x7SEC();
      //Beep_OK();
    }
    //---fSW_G--------------------------------------------------------
    fSW_G = 0;
  }
  
  //Switch Function O
  if(fSW_O){
    if(InputEnable[3] == 1){
      InputCount[3] = InputCount[3]+1;
    }
    
    //---fSW_O--------------------------------------------------------
    if(PickReg[U_LED2]==R_WAITING){
      if(PickReg[U_LED1]==R_INPROCESS){
        PickReg[U_LED1] = R_WAITING;
        LED_Status[U_LED1] = S_ON;
      }
      if(PickReg[U_LED3]==R_INPROCESS){
        PickReg[U_LED3] = R_WAITING;
        LED_Status[U_LED3] = S_ON;
      }
      PickReg[U_LED2] = R_INPROCESS;

      SAVENSEC_Num = PickQty[U_LED2];
      SAVENSEC_Status = S_ON;
      LED_Status[U_LED2] = S_BLING;
      
      putEEPROM_Data();
      Update_4x7SEC();
      //Beep_Enter();
    }else if((PickReg[U_LED2]==R_INPROCESS)){
      PickReg[U_LED2] = R_FINISH;
      SAVENSEC_Num = PickQty[U_LED2];
      SAVENSEC_Status = S_BLING;
      LED_Status[U_LED2] = S_OFF;

      putEEPROM_Data();
      Update_4x7SEC();
      //Beep_OK();
    }
    //---fSW_O--------------------------------------------------------
    fSW_O = 0;
  }

  //Switch Function R
  if(fSW_R){
    if(InputEnable[4] == 1){
      InputCount[4] = InputCount[4]+1;
    }
    
    //---fSW_R--------------------------------------------------------
    if(PickReg[U_LED3]==R_WAITING){
   
      if(PickReg[U_LED2]==R_INPROCESS){
        PickReg[U_LED2] = R_WAITING;
        LED_Status[U_LED2] = S_ON;
      }

      if(PickReg[U_LED1]==R_INPROCESS){
        PickReg[U_LED1] = R_WAITING;
        LED_Status[U_LED1] = S_ON;
      }
      PickReg[U_LED3] = R_INPROCESS;
  
      SAVENSEC_Num = PickQty[U_LED3];
      SAVENSEC_Status = S_ON;
      LED_Status[U_LED3] = S_BLING;
      
      putEEPROM_Data();
      Update_4x7SEC();
      //Beep_Enter();
      
    }else if((PickReg[U_LED3]==R_INPROCESS)){
      PickReg[U_LED3] = R_FINISH;
      SAVENSEC_Num = PickQty[U_LED3];
      SAVENSEC_Status = S_BLING;
      LED_Status[U_LED3] = S_OFF;
      putEEPROM_Data();
      Update_4x7SEC();
      //Beep_OK();
    }
    
    //---fSW_R--------------------------------------------------------
    fSW_R = 0;
  }
  
  //Switch Function F1 and F2
  if(fSW_F1_F2){
      SAVENSEC_Status = S_PRINT_FW;
      SAVENSEC_Num  = FIRMWARE_VS;
      D_control = S_OFF;
      C_control = S_OFF;
      LED9_Status = S_OFF;
      LED_Status[U_LED8] = S_OFF;
      LED_Status[U_LED7] = S_OFF;
      LED_Status[U_LED6] = S_OFF;
      LED_Status[U_LED5] = S_OFF;
      LED_Status[U_LED4] = S_OFF;
      LED_Status[U_LED3] = S_OFF;
      LED_Status[U_LED2] = S_OFF;
      LED_Status[U_LED1] = S_OFF;
      Update_4x7SEC();
      delay(1500);
      getEEPROM_Data();
      Update_4x7SEC();
    
    //---fSW_F1_F2-----------------------------------------------------
    fSW_F1_F2 = 0;
    
  }

  if (CheckUnitRegister()>0){
    SAVENSEC_Status = S_PRINT_ERR;
    SAVENSEC_Num  = Unit_Status;
    Update_4x7SEC();
  }

  if (fByteComplete == true) {
    
    /*RS485_TX_Enable();
    Serial.print((char)IncomingByte[0]);
    Serial.print((char)IncomingByte[1]);
    Serial.print((char)IncomingByte[2]);
    Serial.print((char)IncomingByte[3]);
    Serial.print((char)IncomingByte[4]);
    Serial.print((char)IncomingByte[5]);
    Serial.print((char)IncomingByte[6]);
    Serial.print((char)IncomingByte[7]);
    Serial.print((char)IncomingByte[8]);
    Serial.print((char)IncomingByte[9]);
    Serial.print((char)IncomingByte[10]);
    Serial.print((char)IncomingByte[11]);
    Serial.print((char)IncomingByte[12]);
    Serial.print((char)IncomingByte[13]);
    Serial.print((char)IncomingByte[14]);
    Serial.print((char)IncomingByte[15]);
    Serial.print((char)IncomingByte[16]);
    Serial.print((char)IncomingByte[17]);
    Serial.print((char)IncomingByte[18]);
    Serial.print((char)IncomingByte[19]);
    Serial.println();
    RS485_RX_Enable();*/
   
    Check_CMD();
    Count_IncomingByte = 0;
    for(byte i=0;i<20;i++){  //19->20
      IncomingByte[i]=0;
    }
    fByteComplete = false;
  }

  if (fPrintAddress == true){
    Unit_Address = read_module_address();
    SAVENSEC_Status = S_PRINT_ADDRESS;
    SAVENSEC_Num  = Unit_Address;
    Update_4x7SEC();
    delay(70);
  }
  
  while (Serial.available()) {
    inByte = Serial.read();
    IncomingByte[Count_IncomingByte] = inByte;
    
    if (Count_IncomingByte<20)
      Count_IncomingByte = Count_IncomingByte+1;
    else
      Count_IncomingByte = 0;
      
    if (inByte == CRT) {
      fByteComplete = true;
      
    }
    
  }
        
  RS485_RX_Enable();
  
}

void Check_CMD(){

  byte Input_UnitAddress = 0;
  byte Input_NumLED = 0; // COLOR=>NumLED
  int  Input_Qty = 0;
  int  Input_Bcc = 0;

  char Data_LCD1[10];
  char Data_LCD2[10];
  char Data_LCD3[10];
  char Data_LCD4[10];
  char Data_LCD5[10];
  char Data_LCD6[10];
  char Data_LCD7[10];
  char Data_LCD8[10];
  
  byte CheckSum = 0;
  //int CheckSum = 0;
  byte ReturnValue = 0;
  byte ReturnInpCount = 0;
  
  int IndexOf_ENQ = IndexOfByte(ENQ);//Establish Communication => ENQ
  int IndexOf_EOT = IndexOfByte(EOT);//Cancel Communication => EOT
  int IndexOf_STX = IndexOfByte(STX);//CMD => STX
  int IndexOf_CR  = IndexOfByte(CRT);//End of CMD => CRT

  if(IndexOf_CR >=0){
    if(IndexOf_ENQ >= 0){//Establish Communication 7Byte  
      if((IndexOf_CR-IndexOf_ENQ)==7){
        //Check Unit Address
        Input_UnitAddress = GetNumber_IncomingByte(IndexOf_ENQ+4,3);
        if(Input_UnitAddress == Unit_Address){
          fStartComm = true;
          Hub_Address = GetNumber_IncomingByte(IndexOf_ENQ+1,3);
          
          //Response => Establish Communication
          RS485_TX_Enable();
          //Serial.print("Establish Communication : ");
          Serial.print((char)ACK);
          Serial.print((char)GetChar_Number(Hub_Address,100));
          Serial.print((char)GetChar_Number(Hub_Address,10));
          Serial.print((char)GetChar_Number(Hub_Address,1));
          Serial.print((char)GetChar_Number(Unit_Address,100));
          Serial.print((char)GetChar_Number(Unit_Address,10));
          Serial.print((char)GetChar_Number(Unit_Address,1));
          Serial.println((char)CRT);
          RS485_RX_Enable();
        }
        else{
          fStartComm = false;
          RS485_RX_Enable();
        }
      }
    }
    
    else if(IndexOf_EOT >= 0){//Cancel Communication 2Byte
      if(((IndexOf_CR-IndexOf_EOT)==1)&&(fStartComm == true)){
        //Serial.println("Reset/Cancel Communication");
        fStartComm = false;
        RS485_RX_Enable();
        //Beep_Enter();
      }
    }
    
    else if((IndexOf_STX >= 0)&&(fStartComm == true)){//CMD Communication
      //GET Comd
      CMD_Input = "";
      CMD_Input += (char)IncomingByte[1];
      CMD_Input += (char)IncomingByte[2];
      CMD_Input += (char)IncomingByte[3];
      
      //----------------- CMD_PrintAddress => PAD -----------------ok
      if((CMD_Input.equals(CMD_PrintAddress))&&((IndexOf_CR-IndexOf_STX)==8)){
        //Check Sum
        Input_Bcc = GetNumber_IncomingByte(IndexOf_STX+6,2);
        CheckSum = 0;
        for(byte i=1;i<=4;i++){
          CheckSum = CheckSum+(byte)IncomingByte[i];
        }
        CheckSum = (CheckSum%100);

        //Check Sum => OK
        if(Input_Bcc == CheckSum){
          if(IncomingByte[IndexOf_STX+4] == 'P'){
            fDisableInputSwitch = true;
            fPrintAddress = true;
            SAVENSEC_Status = S_PRINT_ADDRESS;
            SAVENSEC_Num  = Unit_Address;
            D_control = S_OFF;
            C_control = S_OFF;
            LED9_Status = S_OFF;
            LED_Status[U_LED8] = S_OFF;
            LED_Status[U_LED7] = S_OFF;
            LED_Status[U_LED6] = S_OFF;
            LED_Status[U_LED5] = S_OFF;
            LED_Status[U_LED4] = S_OFF;
            LED_Status[U_LED3] = S_OFF;
            LED_Status[U_LED2] = S_OFF;
            LED_Status[U_LED1] = S_OFF;
            Update_4x7SEC();
            ReturnValue = RTV_OK;

          }
          else if(IncomingByte[IndexOf_STX+4] == 'C'){
            fDisableInputSwitch = false;
            fPrintAddress = false;
            getEEPROM_Data();
            Update_4x7SEC();
            ReturnValue = RTV_OK;
          }
          else{
            ReturnValue = RTV_CMD_ERR;
          }
        }else{
          ReturnValue = RTV_CHS_ERR;
        }

        RS485_TX_Enable();
        //Serial.print("Clear Data : ");
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
      }
      //----------------- CMD_PrintAddress => PAD -----------------

      //----------------- CMD_WriteOutputREG => WOR -----------------ok
      else if((CMD_Input.equals(CMD_WriteOutputREG))&&((IndexOf_CR-IndexOf_STX)==9)){
        //Check Sum
        Input_Bcc = GetNumber_IncomingByte(IndexOf_STX+7,2);
        CheckSum = 0;
        for(byte i=1;i<=5;i++){
          CheckSum = CheckSum+(byte)IncomingByte[i];
        }
        CheckSum = (CheckSum%100);
        //Check Sum => OK
        if(Input_Bcc == CheckSum){
          //LED1
          if(IncomingByte[IndexOf_STX+4] == '1'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              RED = S_ON;
              ReturnValue = RTV_OK;
            }else{
              RED = S_OFF;
              ReturnValue = RTV_OK;
            }
            //putEEPROM_Data();
            Update_4x7SEC();
          }
          //LED2
          else if(IncomingByte[IndexOf_STX+4] == '2'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              GREEN = S_ON;
              ReturnValue = RTV_OK;
            }else{
              GREEN = S_OFF;
              ReturnValue = RTV_OK;
            }
            //putEEPROM_Data();
            Update_4x7SEC();
          }
          //LED3
          else if(IncomingByte[IndexOf_STX+4] == '3'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              BLUE = S_ON;
              ReturnValue = RTV_OK;
            }else{
              BLUE = S_OFF;
              ReturnValue = RTV_OK;
            }
            //putEEPROM_Data();
            Update_4x7SEC();

          }
          //LED4 
          else if(IncomingByte[IndexOf_STX+4] == '4'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED_Status[U_LED4] = S_ON;
              ReturnValue = RTV_OK;
            }else{
              LED_Status[U_LED4] = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();

          }
          //LED5 
          else if(IncomingByte[IndexOf_STX+4] == '5'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED_Status[U_LED5] = S_ON;
              ReturnValue = RTV_OK;
            }else{
              LED_Status[U_LED5] = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();

          }
          //LED6 
          else if(IncomingByte[IndexOf_STX+4] == '6'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED_Status[U_LED6] = S_ON;
              ReturnValue = RTV_OK;
            }else{
              LED_Status[U_LED6] = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();

          }
          //LED7 
          else if(IncomingByte[IndexOf_STX+4] == '7'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED_Status[U_LED7] = S_ON;
              ReturnValue = RTV_OK;
            }else{
              LED_Status[U_LED7] = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();

          }
          //LED8 
          else if(IncomingByte[IndexOf_STX+4] == '8'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED_Status[U_LED8] = S_ON;
              ReturnValue = RTV_OK;
            }else{
              LED_Status[U_LED8] = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();

          }
          //LED9
          else if(IncomingByte[IndexOf_STX+4] == '9'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED9_Status = S_ON;
              ReturnValue = RTV_OK;
                }else{
                  LED9_Status = S_OFF;
                  ReturnValue = RTV_OK;
                  }              
            putEEPROM_Data();
            Update_4x7SEC();
        }
          //Out4-12V control
          else if(IncomingByte[IndexOf_STX+4] == 'C'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              C_control = S_ON;
              ReturnValue = RTV_OK;
            }else{
              C_control = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();
          }
          //Out5-D(i/o)
          else if(IncomingByte[IndexOf_STX+4] == 'D'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              //digitalWrite(Out5, 1);
              D_control = S_ON;
              ReturnValue = RTV_OK;
            }else{
              //digitalWrite(Out5, 0);
              D_control = S_OFF;
              ReturnValue = RTV_OK;
            }
            putEEPROM_Data();
            Update_4x7SEC();
          }
          else if(IncomingByte[IndexOf_STX+4] == 'A'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              LED_Status[U_LED1] = S_ON;
              LED_Status[U_LED2] = S_ON;
              LED_Status[U_LED3] = S_ON;
              LED_Status[U_LED4] = S_ON;
              LED_Status[U_LED5] = S_ON;
              LED_Status[U_LED6] = S_ON;
              LED_Status[U_LED7] = S_ON;
              LED_Status[U_LED8] = S_ON;
              LED9_Status = S_ON;
              C_control = S_ON;
              D_control = S_ON;
              RED = S_ON;
              GREEN = S_ON;
              BLUE = S_ON;
              //digitalWrite(Out4, 1);
              //digitalWrite(Out5, 1);
              ReturnValue = RTV_OK;
            }else{
              LED_Status[U_LED1] = S_OFF;
              LED_Status[U_LED2] = S_OFF;
              LED_Status[U_LED3] = S_OFF;
              LED_Status[U_LED4] = S_OFF;
              LED_Status[U_LED5] = S_OFF;
              LED_Status[U_LED6] = S_OFF;
              LED_Status[U_LED7] = S_OFF;
              LED_Status[U_LED8] = S_OFF;
              LED9_Status = S_OFF;
              C_control = S_OFF;
              D_control = S_OFF;
              RED = S_OFF;
              GREEN = S_OFF;
              BLUE = S_OFF;
              //digitalWrite(Out4, 0);
              //digitalWrite(Out5, 0);
              ReturnValue = RTV_OK;
            }
            Update_4x7SEC();
            
          }else{
            
            ReturnValue = RTV_CMD_ERR; //243
          }
        }else{
          
          ReturnValue = RTV_CHS_ERR; //241
        }

        RS485_TX_Enable();
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
      }
      //----------------- CMD_WriteOutputREG => WOR -----------------

      //----------------- CMD_EnableDisableInput => EDI -----------------
      else if((CMD_Input.equals(CMD_EnableDisableInput))&&((IndexOf_CR-IndexOf_STX)==9)){
        //Check Sum
        Input_Bcc = GetNumber_IncomingByte(IndexOf_STX+7,2);
        CheckSum = 0;
        for(byte i=1;i<=5;i++){
          CheckSum = CheckSum+(byte)IncomingByte[i];
                    /*RS485_TX_Enable();
                    Serial.print("i");
                    Serial.print(i);
                    Serial.print("  IncomingByte");
                    Serial.print(IncomingByte[i]);
                    Serial.print("  Check_Sum");
                    Serial.print(CheckSum);
                    Serial.println();
                    RS485_RX_Enable();*/
        }
        CheckSum = (CheckSum%100);

        //Check Sum => OK
        if(Input_Bcc == CheckSum){
          //F1
          if(IncomingByte[IndexOf_STX+4] == '1'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              InputEnable[0] = 1;
              ReturnValue = RTV_OK;
            }else{
              InputEnable[0] = 0;
              ReturnValue = RTV_OK;
            }
          }
          //F2
          else if(IncomingByte[IndexOf_STX+4] == '2'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              InputEnable[1] = 1;
              ReturnValue = RTV_OK;
            }else{
              InputEnable[1] = 0;
              ReturnValue = RTV_OK;
            }
          }
          //Inp1
          else if(IncomingByte[IndexOf_STX+4] == '3'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              InputEnable[2] = 1;
              ReturnValue = RTV_OK;
            }else{
              InputEnable[2] = 0;
              ReturnValue = RTV_OK;
            }
          }
          
          //Inp 2
          else if(IncomingByte[IndexOf_STX+4] == '4'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              InputEnable[3] = 1;
              ReturnValue = RTV_OK;
            }else{
              InputEnable[3] = 0;
              ReturnValue = RTV_OK;
            }
          }
          //Inp 3
          else if(IncomingByte[IndexOf_STX+4] == '5'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              InputEnable[4] = 1;
              ReturnValue = RTV_OK;
            }else{
              InputEnable[4] = 0;
              ReturnValue = RTV_OK;
            }
          }
          else if(IncomingByte[IndexOf_STX+4] == 'A'){
            if(IncomingByte[IndexOf_STX+5] == '1'){
              InputEnable[0] = 1;
              InputEnable[1] = 1;
              InputEnable[2] = 1;
              InputEnable[3] = 1;
              InputEnable[4] = 1;
              ReturnValue = RTV_OK;
            }else{
              InputEnable[0] = 0;
              InputEnable[1] = 0;
              InputEnable[2] = 0;
              InputEnable[3] = 0;
              InputEnable[4] = 0;
              ReturnValue = RTV_OK;
            }
          }
          else{
            ReturnValue = RTV_CMD_ERR;//243
          }
        }else{
          ReturnValue = RTV_CHS_ERR;//241
        }

        RS485_TX_Enable();
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
      }
      //----------------- CMD_EnableDisableInput => EDI -----------------

      //----------------- CMD_ReadInputREG => RIR -----------------
      else if((CMD_Input.equals(CMD_ReadInputREG))&&((IndexOf_CR-IndexOf_STX)==8)){
        //Check Sum
        Input_Bcc = GetNumber_IncomingByte(IndexOf_STX+6,2);
        CheckSum = 0;
        for(byte i=1;i<=4;i++){
          CheckSum = CheckSum+(byte)IncomingByte[i];
                    /*RS485_TX_Enable();
                    Serial.print("i");
                    Serial.print(i);
                    Serial.print("  IncomingByte");
                    Serial.print(IncomingByte[i]);
                    Serial.print("  Check_Sum");
                    Serial.print(CheckSum);
                    Serial.println();
                    RS485_RX_Enable();*/
        }
        CheckSum = (CheckSum%100);

        //Check Sum => OK
        if(Input_Bcc == CheckSum){
          if(IncomingByte[IndexOf_STX+4] == '1'){
            ReturnInpCount = InputCount[0];
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '2'){
            ReturnInpCount = InputCount[1];
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '3'){
            ReturnInpCount = InputCount[2];
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '4'){
            ReturnInpCount = InputCount[3];
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '5'){
            ReturnInpCount = InputCount[4];
            ReturnValue = RTV_OK;
          }
          else{
            ReturnValue = RTV_CMD_ERR;//243
          }
        }else{
          ReturnValue = RTV_CHS_ERR;//241
        }

        RS485_TX_Enable();
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.print((char)GetChar_Number(ReturnInpCount,100));
        Serial.print((char)GetChar_Number(ReturnInpCount,10));
        Serial.print((char)GetChar_Number(ReturnInpCount,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
      }
      //----------------- CMD_ReadInputREG => RIR -----------------

      //----------------- CMD_ClearInputREG => CIR -----------------
      else if((CMD_Input.equals(CMD_ClearInputREG))&&((IndexOf_CR-IndexOf_STX)==8)){
        //Check Sum
        Input_Bcc = GetNumber_IncomingByte(IndexOf_STX+6,2);
        CheckSum = 0;
        for(byte i=1;i<=4;i++){
          CheckSum = CheckSum+(byte)IncomingByte[i];
        }
        CheckSum = (CheckSum%100);

        //Check Sum => OK
        if(Input_Bcc == CheckSum){
          if(IncomingByte[IndexOf_STX+4] == '1'){
            InputCount[0] = 0;
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '2'){
            InputCount[1] = 0;
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '3'){
            InputCount[2] = 0;
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '4'){
            InputCount[3] = 0;
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == '5'){
            InputCount[4] = 0;
            ReturnValue = RTV_OK;
          }
          else if(IncomingByte[IndexOf_STX+4] == 'A'){
            InputCount[0] = 0;
            InputCount[1] = 0;
            InputCount[2] = 0;
            InputCount[3] = 0;
            InputCount[4] = 0;
            ReturnValue = RTV_OK;
          }
          else{
            ReturnValue = RTV_CMD_ERR;//243
          }
        }else{
          ReturnValue = RTV_CHS_ERR;//241
        }

        RS485_TX_Enable();
        //Serial.print("Clear Data : ");
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
      }
      //----------------- CMD_ClearInputREG => CIR -----------------
      
      //----------------- CMD_WriteLCD => LCD ----------------------
        else if((CMD_Input.equals(CMD_WriteLCD))&&((IndexOf_CR-IndexOf_STX)==18)){ //LCD 19 byte
        Input_Bcc = GetNumber_IncomingByte(IndexOf_STX+16,2);
        CheckSum = 0;
        for(byte i=1;i<=14;i++){ // SUM 
          CheckSum = CheckSum+(byte)IncomingByte[i];
          }
                    
          CheckSum = (CheckSum%100);
          
          if(Input_Bcc == CheckSum){
            if(IncomingByte[IndexOf_STX+4] == '1'){ // STX+4 => Position LCD 1-8
                for (byte i=0;i<10;i++){
                  Data_LCD1[i] = IncomingByte[IndexOf_STX+(5+i)];

                    /*RS485_TX_Enable();
                    Serial.print("Income");
                    Serial.print("=>");
                    Serial.print(IncomingByte[IndexOf_STX+(5+i)]);
                    Serial.print(" || ");
                    Serial.print(i);
                    Serial.print(" =>");
                    Serial.print(Data_LCD1[i]);
                    Serial.println();
                    RS485_RX_Enable();*/
                    
                    lcd.setCursor(i,0);
                    lcd.print(Data_LCD1[i]);
                    delay(1);
                      
                    ReturnValue = RTV_OK;
                }
            }
            else  if(IncomingByte[IndexOf_STX+4] == '2'){
                for (byte i=0;i<10;i++){
                  Data_LCD2[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(10+i,0);
                    lcd.print(Data_LCD2[i]);
                    delay(1);
                    
                    ReturnValue = RTV_OK;
                }
              }
              else  if(IncomingByte[IndexOf_STX+4] == '3'){
                for (byte i=0;i<10;i++){
                  Data_LCD3[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(i,1);
                    lcd.print(Data_LCD3[i]);
                    delay(1);
                    
                    ReturnValue = RTV_OK;
                }
              }
               else  if(IncomingByte[IndexOf_STX+4] == '4'){
                for (byte i=0;i<10;i++){
                  Data_LCD4[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(10+i,1);
                    lcd.print(Data_LCD4[i]);
                    delay(1);
                    
                    ReturnValue = RTV_OK;
                }
              }
               else  if(IncomingByte[IndexOf_STX+4] == '5'){
                for (byte i=0;i<10;i++){
                  Data_LCD5[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(i,2);
                    lcd.print(Data_LCD5[i]);
                    delay(1);
                    
                    ReturnValue = RTV_OK;
                }
              }
               else  if(IncomingByte[IndexOf_STX+4] == '6'){
                for (byte i=0;i<10;i++){
                  Data_LCD6[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(10+i,2);
                    lcd.print(Data_LCD6[i]);
                    delay(1);
                    
                    ReturnValue = RTV_OK;
                }
              }
              else  if(IncomingByte[IndexOf_STX+4] == '7'){
                for (byte i=0;i<10;i++){
                  Data_LCD7[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(i,3);
                    lcd.print(Data_LCD7[i]);
                    delay(1);
                    
                    ReturnValue = RTV_OK;
                }
              }
              else  if(IncomingByte[IndexOf_STX+4] == '8'){
                for (byte i=0;i<10;i++){
                  Data_LCD8[i] = IncomingByte[IndexOf_STX+(5+i)];
                    
                    lcd.setCursor(10+i,3);
                    lcd.print(Data_LCD8[i]);
                    delay(1);
 
                    ReturnValue = RTV_OK;
                }
              }
              else if(IncomingByte[IndexOf_STX+4] == 'A'){
                for(byte i=0;i<10;i++){
                  Data_LCD1[i] = 0;
                    lcd.setCursor(i,0);
                    lcd.clear();
                    delay(1);
                    
                  Data_LCD2[i] = 0;
                    lcd.setCursor(10+i,0);
                    lcd.clear();
                    delay(1);
                    
                  Data_LCD3[i] = 0;
                    lcd.setCursor(i,1);
                    lcd.clear();
                    delay(1);
                    
                  Data_LCD4[i] = 0;
                    lcd.setCursor(10+i,1);
                    lcd.clear();
                    delay(1);
                    
                  Data_LCD5[i] = 0;
                    lcd.setCursor(i,2);
                    lcd.clear();
                    delay(1);
                    
                  Data_LCD6[i] = 0;
                    lcd.setCursor(10+i,2);
                    lcd.clear();
                    delay(1);
                    
                  Data_LCD7[i] = 0;
                    lcd.setCursor(i,3);
                    lcd.print(Data_LCD7[i]);
                    delay(1);
                    
                  Data_LCD8[i] = 0;
                    lcd.setCursor(10+i,3);
                    lcd.clear();
                    delay(1);
                    
                  }
                  ReturnValue = RTV_OK;
                }
              else{
            ReturnValue = RTV_CMD_ERR;//243
          }
            
          }else{
          ReturnValue = RTV_CHS_ERR;//241
        }
        RS485_TX_Enable();
        //Serial.print("Charecter : ");
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
        }
  
        
        //---------------------------------------------------------
        
        //--------------------- CMD_ERR ---------------------------
      else{
        ReturnValue = RTV_CMD_ERR;
        RS485_TX_Enable();
        Serial.print((char)ACK);
        Serial.print((char)GetChar_Number(ReturnValue,100));
        Serial.print((char)GetChar_Number(ReturnValue,10));
        Serial.print((char)GetChar_Number(ReturnValue,1));
        Serial.println((char)CRT);
        RS485_RX_Enable();
        
      }
          
    }
  }
}

int GetNumber_IncomingByte(byte Start,byte len){

  int ret = 0;
  int temp = 0;

  if (len == 4){
    temp = ConvertDigit2Byte(IncomingByte[Start+0]);
    if (temp >= 0)
      ret = ret+temp*1000;
    else
      return -1;
      
    temp = ConvertDigit2Byte(IncomingByte[Start+1]);
    if (temp >= 0)
      ret = ret+temp*100;
    else
      return -1;
      
    temp = ConvertDigit2Byte(IncomingByte[Start+2]);
    if (temp >= 0)
      ret = ret+temp*10;
    else
      return -1;
      
    temp = ConvertDigit2Byte(IncomingByte[Start+3]);
    if (temp >= 0)
      ret = ret+temp*1;
    else
      return -1;
      
  }else if(len == 3){
    temp = ConvertDigit2Byte(IncomingByte[Start+0]);
    if (temp >= 0)
      ret = ret+temp*100;
    else
      return -1;
    temp = ConvertDigit2Byte(IncomingByte[Start+1]);
    if (temp >= 0)
      ret = ret+temp*10;
    else
      return -1;
    temp = ConvertDigit2Byte(IncomingByte[Start+2]);
    if (temp >= 0)
      ret = ret+temp*1;
    else
      return -1;
      
  }else if(len == 2){
    temp = ConvertDigit2Byte(IncomingByte[Start+0]);
    if (temp >= 0)
      ret = ret+temp*10;
    else
      return -1;
    temp = ConvertDigit2Byte(IncomingByte[Start+1]);
    if (temp >= 0)
      ret = ret+temp*1;
    else
      return -1;
  }
  
  return ret;
}

byte GetChar_Number(int Num,int index){
    return ((Num/index)%10)+'0';
}

int ConvertDigit2Byte(byte conv){
  if (isDigit(conv))
    return (conv-0x30);
  else
    return -1;
}

int IndexOfByte(byte DataSearch){
  for(byte i=0 ; i<Count_IncomingByte ; i++){
    if(IncomingByte[i]==DataSearch){
      return i;
    }
  }
  return -1;
}

void ResetUnitRegister(){
  
  PickReg[2] = R_FREE;
  PickQty[2] = 0;
  
  PickReg[1] = R_FREE;
  PickQty[1] = 0;
  
  PickReg[0] = R_FREE;
  PickQty[0] = 0;
  
  SAVENSEC_Num = 0;
  SAVENSEC_Status = S_BLING;
  D_control = S_OFF;
  C_control = S_OFF;
  LED9_Status = S_OFF;
  LED_Status[U_LED8] = S_OFF;
  LED_Status[U_LED7] = S_OFF;
  LED_Status[U_LED6] = S_OFF;
  LED_Status[U_LED5] = S_OFF;
  LED_Status[U_LED4] = S_OFF;
  LED_Status[U_LED3] = S_OFF;
  LED_Status[U_LED2] = S_OFF;
  LED_Status[U_LED1] = S_OFF;
  
  Update_4x7SEC();
  
  Unit_Status = RTV_OK;
  
}

byte CheckUnitRegister(){
  //clearEEPROM_Data();
  //printEEPROM_Data(0);
  //printEEPROM_Data(1);
  //printEEPROM_Data(2);
  //printEEPROM_Data(3);
  //printEEPROM_Data(4);
  //printEEPROM_Data(5);
  //printEEPROM_Data(6);
  //printEEPROM_Data(7);
  byte RetStatus = 0;
  
  for(byte i=0;i<3;i++){
    if(PickQty[i] > 9999)
      RetStatus = RetStatus+1;
      
    if(PickReg[i] > 3) //R_FINISH = 3----------------------------------------------
      RetStatus = RetStatus+1;

    /*RS485_TX_Enable();
    Serial.print(i);
    Serial.print("//Qty->");
    Serial.print(PickQty[i]);
    Serial.println();
    Serial.print(i);
    Serial.print("//Reg->");
    Serial.print(PickReg[i]);
    Serial.println();
    RS485_RX_Enable();*/
    
  }

  if (RetStatus > 0)
    Unit_Status = RTV_REG_ERR;
  else 
    Unit_Status = RTV_OK;
    
  return(RetStatus);
  
}

byte CheckSwitchSignal(){
  
  if(fSW_F1==0){
    if(digitalRead(SW_F1)){
      if (countSW_F1>X150MS){
        //fSW_F1 = 1;
        //countSW_F1 = 0;
      }else{
        countSW_F1 = countSW_F1+1;
      }
    }else if(countSW_F1>X50MS){
      fSW_F1 = 1;
      countSW_F1 = 0;
    }
  }
  //if((fSW_F2==0)&&(fDisableInputSwitch == false)){
  if(fSW_F2==0){
    if(digitalRead(SW_F2)){
      if(countSW_F2>X150MS){
        //fSW_F2 = 1;
        //countSW_F2 = 0;
      }else{
        countSW_F2 = countSW_F2+1;
      }
    }else if(countSW_F2>X50MS){
      fSW_F2 = 1;
      countSW_F2 = 0;
    }
  }
  if((fSW_F1_F2==0)&&(fDisableInputSwitch == false)){
    if((digitalRead(SW_F1)==1)&&(digitalRead(SW_F2)==1)){
      if (countSW_F1_F2>(X150MS+X150MS)){
        //fSW_F1_F2 = 1;
        //countSW_F1_F2 = 0;
      }else{
        countSW_F1_F2 = countSW_F1_F2+1;
      }
    }else if(countSW_F1_F2>X50MS){
      fSW_F1_F2 = 1;
      countSW_F1_F2 = 0;
    }
  }
  
  //if((fSW_G==0)&&(fDisableInputSwitch == false)){
    if(fSW_G==0){
    if(digitalRead(SW_G)){
      if(countSW_G>X150MS){
        //fSW_G = 1;
        //countSW_G = 0;
      }else{
        countSW_G = countSW_G+1;
      }
    }else if(countSW_G>X50MS){
      fSW_G = 1;
      countSW_G = 0;
    }
  }
  
  //if((fSW_O==0)&&(fDisableInputSwitch == false)){
    if(fSW_O==0){
    if(digitalRead(SW_O)){
      if(countSW_O>X150MS){
        //fSW_O = 1;
        //countSW_O = 0;
      }else{
        countSW_O = countSW_O+1;
      }
    }else if(countSW_O>X50MS){
      fSW_O = 1;
      countSW_O = 0;
    }
  }
  
  //if((fSW_R==0)&&(fDisableInputSwitch == false)){
    if(fSW_R==0){
    if(digitalRead(SW_R)){
      if(countSW_R>X150MS){
        //fSW_R = 1;
        //countSW_R = 0;
      }else{
        countSW_R = countSW_R+1;
      }
    }else if(countSW_R>X50MS){
      fSW_R = 1;
      countSW_R = 0;
    }
  }
}

void RS485_RX_Enable(void){
  Serial.flush();
  digitalWrite(RS485_DIR, RS485_RX);
  delay(5);
}

void RS485_TX_Enable(void){
  digitalWrite(RS485_DIR, RS485_TX);
  delay(5);
}

void Update_4x7SEC(){
  if(SAVENSEC_Status == S_ON){
    DigitWrite[0] = (SAVENSEC_Num/   1)%10;
    DigitWrite[1] = (SAVENSEC_Num/  10)%10;
    DigitWrite[2] = (SAVENSEC_Num/ 100)%10;
    DigitWrite[3] = (SAVENSEC_Num/1000)%10;
  }else if(SAVENSEC_Status == S_PRINT_ADDRESS){
    DigitWrite[0] = (SAVENSEC_Num/   1)%10;
    DigitWrite[1] = (SAVENSEC_Num/  10)%10;
    DigitWrite[2] = (SAVENSEC_Num/ 100)%10;
    DigitWrite[3] = 0x0A;
  }else if(SAVENSEC_Status == S_PRINT_ERR){
    DigitWrite[0] = (SAVENSEC_Num/   1)%10;
    DigitWrite[1] = (SAVENSEC_Num/  10)%10;
    DigitWrite[2] = (SAVENSEC_Num/ 100)%10;
    DigitWrite[3] = 0x0E;
  }else if(SAVENSEC_Status == S_PRINT_FW){
    DigitWrite[0] = (SAVENSEC_Num/   1)%10;
    DigitWrite[1] = (SAVENSEC_Num/  10)%10;
    DigitWrite[2] = (SAVENSEC_Num/ 100)%10;
    DigitWrite[3] = 0x0F;
  }
  else{
    DigitWrite[0] = 16;
    DigitWrite[1] = 16;
    DigitWrite[2] = 16;
    DigitWrite[3] = 16;
  }
}

void Print_4X7SEC(){
  int y=5;
  byte LEDA = 0;
  byte LEDB = 0;
  byte Digit_LED = 0;
  /*for(byte i=0; i<U_TOTAL; i++){ //i=U_TOTAL

    if(LED_Status[i] == 0){
      bitWrite(DigitWrite[6], i, LOW);
    }else if(LED_Status[i] == 1){
      bitWrite(DigitWrite[6], i, HIGH);
      } else{
        bitWrite(DigitWrite[6], i, fbling_count1);
        }

    //RS485_TX_Enable();
    //Serial.print(i);
    //Serial.print("//");
    //Serial.print(LED_Status[i]);
    //Serial.println();
    //RS485_RX_Enable();
  }*/
      if(RED == 1){
        //analogWrite(U_LED1, 0);
        digitalWrite(U_LED1, 0);
        }else{
          //analogWrite(U_LED1, 255);
          digitalWrite(U_LED1, 1);
          }

        if(GREEN == 1){
        //analogWrite(U_LED2, 0);
        digitalWrite(U_LED2, 0);
        }else{
          //analogWrite(U_LED2, 255);
          digitalWrite(U_LED2, 1);
          }

        if(BLUE == 1){
        //analogWrite(U_LED3, 0);
        digitalWrite(U_LED3, 0);
        }else{
          //analogWrite(U_LED3, 255);
          digitalWrite(U_LED3, 1);
          }
  
          
    if (SAVENSEC_Status==2){  
      DigitWrite[0] = 16+fbling_count2; 
      
  }
  
    /*if(LED9_Status == 1){
      //LEDA = 1<<y;
      LEDA = 0x20;
      Digit_LED = (Digit[digit_count]|LEDA);
     }else if(LED9_Status == 0){
      //LEDB = 0<<y;
      LEDB = 0xDF;
      Digit_LED = (Digit[digit_count]&LEDB);
      }*/

      /*if(C_control == 1){
        digitalWrite(Out4, 1);
        }else{
          digitalWrite(Out4, 0);
          }*/

          /*if(D_control == 1){
            digitalWrite(Out5, 1);
            }else{
              digitalWrite(Out5, 0);
              }*/
    //RS485_TX_Enable();
    //Serial.print(5);
    //Serial.print("//");
    //Serial.print(DigitWrite[5]);
    //Serial.println();
    //RS485_RX_Enable();

  //digitalWrite(LATCHPIN_595, 0);
  //shiftOut(DigitWrite[6]);
  //shiftOut(Digit_LED);
  //shiftOut(CharDecode[DigitWrite[digit_count]]);
  //digitalWrite(LATCHPIN_595, 1);


//  if(LED_Status[U_LED1]==0){
//    digitalWrite(DigitalOut, 0);
//  }else if(LED_Status[U_LED1]==1){
//    digitalWrite(DigitalOut, 1);
//  }else{
//    digitalWrite(DigitalOut, fbling_count1);
//  }

}



/*void shiftOut(byte DataOut){
  int i=0;
  bool pinState;
  digitalWrite(DATAPIN_595, 0);
  digitalWrite(CLOCKPIN_595, 0);
  for (i=7; i>=0; i--){
    digitalWrite(CLOCKPIN_595, 0);
    if ( DataOut & (1<<i) ) 
      pinState= 1;
    else
      pinState= 0;
    
    digitalWrite(DATAPIN_595, pinState);
    digitalWrite(CLOCKPIN_595, 1);
    digitalWrite(DATAPIN_595, 0);
  }
  digitalWrite(CLOCKPIN_595, 0);
}*/

/*void Control_4X7SEC(bool Status){
  if (Status == 0){
    digitalWrite(OUTPUTEN_595, HIGH);
  }else{
    digitalWrite(OUTPUTEN_595, LOW);    
  }
}*/

byte read_module_address(){ 
    byte bitVal;
    byte bytesVal = 0;
    
    digitalWrite(PLOADPIN_165, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(PLOADPIN_165, HIGH);

    for(int i = 0; i < DATA_WIDTH; i++){
        bitVal = digitalRead(DATAPIN_165);
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));
        digitalWrite(CLOCKPIN_165, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(CLOCKPIN_165, LOW);
    }
    return(bytesVal);
}

/*void Beep_OK(void){
  for (byte k = 0; k < 30; k++) {
    digitalWrite(Buzzer,HIGH);
    delayMicroseconds(120);
    digitalWrite(Buzzer,LOW);
    delayMicroseconds(120);
  }
}

void Beep_Enter(void){
  for (byte k = 0; k < 30; k++) {
    digitalWrite(Buzzer,HIGH);
    delayMicroseconds(90);
    digitalWrite(Buzzer,LOW);
    delayMicroseconds(90);
  }
}*/

bool getEEPROM_Data(){

      for(byte i=0;i<U_TOTAL;i++){
      if(LED_Status[i]==0||LED_Status[i]==1){ //3 > Bling
        EEPROM.get(Add_LED_Status[i],LED_Status[i]);
        }
      }
      for(byte i=0;i<3;i++){
        if(PickQty[i]<9999){
          EEPROM.get(Add_PickQty[i],PickQty[i]);
          }
        if(PickReg[i]<3){
          EEPROM.get(Add_PickReg[i],PickReg[i]);
          }
        }
        for(byte i=0;i<1;i++){
          EEPROM.get(Add_LED9_Status,LED9_Status);
          }
          for(byte i=0;i<1;i++){
          EEPROM.get(Add_C_control,C_control);
          }
          for(byte i=0;i<1;i++){
          EEPROM.get(Add_D_control,D_control);
          }
          /*for(byte i=0;i<1;i++){
          EEPROM.get(Add_RED,RED);
          }
          for(byte i=0;i<1;i++){
          EEPROM.get(Add_GREEN,GREEN);
          }
          for(byte i=0;i<1;i++){
          EEPROM.get(Add_BLUE,BLUE);
          }*/

  EEPROM.get(Add_SAVENSEC_Num,SAVENSEC_Num);
  EEPROM.get(Add_SAVENSEC_Status,SAVENSEC_Status);

  return 1;
  
}

bool putEEPROM_Data(){

      for(byte i=0;i<U_TOTAL;i++){
      if(LED_Status[i]==0||LED_Status[i]==1){ //3 > Bling
        EEPROM.put(Add_LED_Status[i],LED_Status[i]);
        }
      }
      for(byte i=0;i<3;i++){
        if(PickQty[i]<9999){
          EEPROM.put(Add_PickQty[i],PickQty[i]);
          }
        if(PickReg[i]<3){
          EEPROM.put(Add_PickReg[i],PickReg[i]);
          }
        }
        for(byte i=0;i<1;i++){
          EEPROM.put(Add_LED9_Status,LED9_Status);
          }
          for(byte i=0;i<1;i++){
          EEPROM.put(Add_C_control,C_control);
          }
          for(byte i=0;i<1;i++){
          EEPROM.put(Add_D_control,D_control);
          }
          /*for(byte i=0;i<1;i++){
          EEPROM.put(Add_RED,RED);
          }
          for(byte i=0;i<1;i++){
          EEPROM.put(Add_GREEN,GREEN);
          }
          for(byte i=0;i<1;i++){
          EEPROM.put(Add_BLUE,BLUE);
          }*/

  EEPROM.put(Add_SAVENSEC_Num,SAVENSEC_Num);
  EEPROM.put(Add_SAVENSEC_Status,SAVENSEC_Status);
  
  return 1;
  
}

bool clearEEPROM_Data(){
  for(byte i=0;i<3;i++){
    PickQty[i] = 0x0000;
    PickReg[i] = 0x00;
    EEPROM.put(Add_PickQty[i],PickQty[i]);
    EEPROM.put(Add_PickReg[i],PickReg[i]);
  }
  for(byte i=0;i<U_TOTAL;i++){
    LED_Status[i] = 0x0;
    EEPROM.put(Add_LED_Status[i],LED_Status[i]);
      }
      for(byte i=0;i<1;i++){
        LED9_Status = 0;
          EEPROM.put(Add_LED9_Status,LED9_Status);
          }
          for(byte i=0;i<1;i++){
          C_control = 0;
          EEPROM.put(Add_C_control,C_control);
          }
          for(byte i=0;i<1;i++){
          D_control = 0;
          EEPROM.put(Add_D_control,D_control);
          }
          /*for(byte i=0;i<1;i++){
          RED = 0;
          EEPROM.put(Add_RED,RED);
          }
          for(byte i=0;i<1;i++){
          GREEN = 0;
          EEPROM.put(Add_GREEN,GREEN);
          }
          for(byte i=0;i<1;i++){
          BLUE = 0;
          EEPROM.put(Add_BLUE,BLUE);
          }*/
  return 1;
  
}

void printEEPROM_Data(byte no){
  Serial.print("------ EEPROM_Data_");
  Serial.print(no);
  Serial.print(" ------");
  Serial.print("\r\n");
  for(byte i=0;i<3;i++){
    Serial.print(" i:");
    Serial.print(i);
    Serial.print(" Add_PickQty:");
    Serial.print(Add_PickQty[i]);
    Serial.print(" / PickQty:");
    Serial.print(PickQty[i]);
    Serial.print(" / Add_PickReg:");
    Serial.print(Add_PickReg[i]);
    Serial.print(" / PickReg:");
    Serial.print(PickReg[i]);
    Serial.print("\r\n");
  }
  for(byte i=0;i<U_TOTAL;i++){
    Serial.print(" i:");
    Serial.print(i);
    Serial.print(" / LED_Status:");
    Serial.print(LED_Status[i]);
    }
  Serial.print("-------------------------------");
  Serial.print("\r\n");
  
}

bool timer1Isr(void){
  if (digit_count<5) 
    digit_count = digit_count+1;
   else
    digit_count = 0;
  
  if (bling_count1<X100MS){
    bling_count1 = bling_count1+1;
  }
  else{
    bling_count1 = 0;
    fbling_count1 = !fbling_count1;
  }
  
  if (bling_count2<X1000MS){
    bling_count2 = bling_count2+1;
  }
  else{
    bling_count2 = 0;
    fbling_count2 = !fbling_count2;
  }

  CheckSwitchSignal(); 
  Print_4X7SEC();

}
