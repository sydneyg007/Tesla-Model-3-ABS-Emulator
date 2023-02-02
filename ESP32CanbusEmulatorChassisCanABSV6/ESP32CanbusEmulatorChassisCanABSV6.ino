//V4 modifying 0x145 byte 7,8 and checksum. 0x185 Adding brake torque (was zero). Fixede 0x175 checksum calc. 
    //0x325 timing corrected. Added 0x3D5 alert matrix. Removed start frame delay
//ver=5; added led on indicator to pin 32
//ver=6; 
          //started 0x175 couner at 0x10 and checksum at 0x86,(fixed 0x175 checksum
          //0x145 modified bytes so no alerts?
          //0x155 first 2 frame different (initialise),also changed main frames
          //0x185 changed (first frames start at all zero - may need to modify when brakes applied
          //0x3D5 fixed error in index 2
          //0x325 change index order


//CanTX pin - gpio 22
//CanRX pin - gpio 23

#include <esp32_can.h>
       
byte counter0x175 = 0x10; //(0x00 to 0xF0, increment by 0x10)
byte checksum0x175 = 0x86; //byte 6 +0x76

byte counter0x145 = 0x41; //increment by + 1
byte checksum0x145 = 0xFC; // increment by + 1

byte frameCount0x155 = 0; 
byte counter0x155 = 0x30; //(0x00 to 0xF0, increment by 0x10 (16))
byte checksum0x155 = 0x86; //(0x03 to F3) counter0x155+0x10;

byte counter0x185 = 0x14; // increment by 0x10 )
byte checksum0x185 = 0x9A; //increment by 0x10  )

byte index0x3D5 = 0x00; //5 indexes

byte index0x325 = 1; //6 indexes

CAN_FRAME outframe;

void Frames20MS()
{
static unsigned long timer20ms = millis();   

if (millis() - timer20ms >= 20) 
  {
  timer20ms=millis();
  
  //0x145(ESP Status)
          outframe.id = 0x145;            // Set our transmission address ID
          outframe.length = 8;            // Data payload 8 bytes
          outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
          outframe.rtr=0;                 //No request
          outframe.data. uint8 [0]= checksum0x145;
          outframe.data. uint8 [1]= counter0x145;
          outframe.data. uint8 [2]= 0x00;
          outframe.data. uint8 [3]= 0xCF;
          outframe.data. uint8 [4]= 0xA4;
          outframe.data. uint8 [5]= 0x00;
          outframe.data. uint8 [6]= 0x02;
          outframe.data. uint8 [7]= 0x00;
          Can0.sendFrame(outframe);         
            checksum0x145++;  
            if (checksum0x145>0xFF)
              {checksum0x145=0x00;}
            counter0x145 ++;
            if (counter0x145 > 0x4F)
              {counter0x145=0x40; checksum0x145=0xFB;}
  
  //0x155 // initial 2 frames are different
  if (frameCount0x155==1)
    {outframe.id = 0x155;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]=0xFF;
    outframe.data. uint8 [1]=0xFF;
    outframe.data. uint8 [2]=0xFF;
    outframe.data. uint8 [3]=0xFF;
    outframe.data. uint8 [4]=0xFF;
    outframe.data. uint8 [5]=0x01;
    outframe.data. uint8 [6]=0x20;
    outframe.data. uint8 [7]=0x72;
    Can0.sendFrame(outframe);
    frameCount0x155++;
    }
    
  if (frameCount0x155==0)
    {outframe.id = 0x155;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]=0xFF;
    outframe.data. uint8 [1]=0xFF;
    outframe.data. uint8 [2]=0xFF;
    outframe.data. uint8 [3]=0xFF;
    outframe.data. uint8 [4]=0xFF;
    outframe.data. uint8 [5]=0xFC;
    outframe.data. uint8 [6]=0x1F;
    outframe.data. uint8 [7]=0x6C;
    Can0.sendFrame(outframe);
    frameCount0x155++;
    }

  if (frameCount0x155>1)
    {outframe.id = 0x155;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]=0x00;
    outframe.data. uint8 [1]=0x00;
    outframe.data. uint8 [2]=0x00;
    outframe.data. uint8 [3]=0x00;
    outframe.data. uint8 [4]=0xFF;
    outframe.data. uint8 [5]=0x01;
    outframe.data. uint8 [6]=counter0x155;
    outframe.data. uint8 [7]=checksum0x155;
    Can0.sendFrame(outframe); 
    counter0x155=counter0x155+0x10;
    if (counter0x155>0xF0)
      {counter0x155=0x00;}
    checksum0x155=checksum0x155+0x10;
    if (checksum0x155>0xF6)
      {checksum0x155=0x06;}
    }

//0x175 ESP_wheelSpeed
      outframe.id = 0x175;            // Set our transmission address ID
      outframe.length = 8;            // Data payload 8 bytes
      outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
      outframe.rtr=0;                 //No request
      outframe.data. uint8 [0]=0x00;
      outframe.data. uint8 [1]=0x00;
      outframe.data. uint8 [2]=0x00;
      outframe.data. uint8 [3]= 0x00;
      outframe.data. uint8 [4]= 0x00;
      outframe.data. uint8 [5]= 0x00;
      outframe.data. uint8 [6]=counter0x175;
      outframe.data. uint8 [7]=checksum0x175;
      Can0.sendFrame(outframe); 
      //Byte 6 calc
      counter0x175=counter0x175+0x10;
      if (counter0x175>0xF0)
      {counter0x175=0x00;}
      //Byte 7 calc
      checksum0x175=checksum0x175+0x10;
      if (checksum0x175>0xF6)
      {checksum0x175=0x06;}                  
  
  //0x185 (ESP_brakeTorque)
          outframe.id = 0x185;            // Set our transmission address ID
          outframe.length = 8;            // Data payload 8 bytes
          outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
          outframe.rtr=0;                 //No request
          outframe.data. uint8 [0]=0x00;
          outframe.data. uint8 [1]=0x00;
          outframe.data. uint8 [2]=0x00;
          outframe.data. uint8 [3]=0x00;
          outframe.data. uint8 [4]=0x00;
          outframe.data. uint8 [5]=0x00;
          outframe.data. uint8 [6]=counter0x185;
          outframe.data. uint8 [7]=checksum0x185;
          Can0.sendFrame(outframe); 
          counter0x185=counter0x185+0x10;
          if (counter0x185>0xF4)
            {counter0x185=0x04;}
          checksum0x185=checksum0x185+0x10;
          if (checksum0x185>0xFA)
            {checksum0x185=0x0A;}
  }
}//end Frames 20ms-------------------------------------------------------------------

void Frames200MS()
{
static unsigned long timer200ms = millis();   

if (millis() - timer200ms >= 200) 
  {
  timer200ms=millis();
  
  //ESP_AlertMatrix
    outframe.id = 0x3D5;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= index0x3D5;
    outframe.data. uint8 [1]= 0x00;
    outframe.data. uint8 [2]= 0x00;
    outframe.data. uint8 [3]= 0x00;
    outframe.data. uint8 [4]= 0x00;
    outframe.data. uint8 [5]= 0x00;
    outframe.data. uint8 [6]= 0x00;
    outframe.data. uint8 [7]= 0x00;
    Can0.sendFrame(outframe); 
    index0x3D5++;
    if (index0x3D5==5)
      {index0x3D5=0;}
    }
}//end Frames 200ms-------------------------------------------------------------------
      
void Frames1670MS()
{
static unsigned long timer1670ms = millis();
if (millis() - timer1670ms >= 1670) 
  {
  timer1670ms=millis();

  //index 1
  if (index0x325==1)
    {outframe.id = 0x325;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= 0x0A;
    outframe.data. uint8 [1]= 0x01;
    outframe.data. uint8 [2]= 0x00;
    outframe.data. uint8 [3]= 0x00;
    outframe.data. uint8 [4]= 0x00;
    outframe.data. uint8 [5]= 0x00;
    outframe.data. uint8 [6]= 0x41;
    outframe.data. uint8 [7]= 0x00;
    Can0.sendFrame(outframe); 
    }
  
  //index 2
  if (index0x325==2)
    {outframe.id = 0x325;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= 0x0B;
    outframe.data. uint8 [1]= 0x00;
    outframe.data. uint8 [2]= 0x04;
    outframe.data. uint8 [3]= 0x02;
    outframe.data. uint8 [4]= 0x02;
    outframe.data. uint8 [5]= 0x00;
    outframe.data. uint8 [6]= 0x00;
    outframe.data. uint8 [7]= 0x00;
    Can0.sendFrame(outframe);
    } 
  
  //index 3
  if (index0x325==3)
    {outframe.id = 0x325;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= 0x0D;
    outframe.data. uint8 [1]= 0x00;
    outframe.data. uint8 [2]= 0x00;
    outframe.data. uint8 [3]= 0x00;
    outframe.data. uint8 [4]= 0x88;
    outframe.data. uint8 [5]= 0x44;
    outframe.data. uint8 [6]= 0x57;
    outframe.data. uint8 [7]= 0xAB;
    Can0.sendFrame(outframe);
    } 
  //index 4
  if (index0x325==4)
    {outframe.id = 0x325;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= 0x10;
    outframe.data. uint8 [1]= 0x00;
    outframe.data. uint8 [2]= 0x0D;
    outframe.data. uint8 [3]= 0x00;
    outframe.data. uint8 [4]= 0xA8;
    outframe.data. uint8 [5]= 0xA8;
    outframe.data. uint8 [6]= 0xA8;
    outframe.data. uint8 [7]= 0xA8;
    Can0.sendFrame(outframe); 
    }
  
  //index 5
  if (index0x325==5)
    {outframe.id = 0x325;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= 0x14;
    outframe.data. uint8 [1]= 0x05;
    outframe.data. uint8 [2]= 0x00;
    outframe.data. uint8 [3]= 0x00;
    outframe.data. uint8 [4]= 0x45;
    outframe.data. uint8 [5]= 0xF8;
    outframe.data. uint8 [6]= 0x51;
    outframe.data. uint8 [7]= 0xA8;
    Can0.sendFrame(outframe); 
    }
  
  //index 6
  if (index0x325==6)
    {outframe.id = 0x325;            // Set our transmission address ID
    outframe.length = 8;            // Data payload 8 bytes
    outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
    outframe.rtr=0;                 //No request
    outframe.data. uint8 [0]= 0x16;
    outframe.data. uint8 [1]= 0x00;
    outframe.data. uint8 [2]= 0x00;
    outframe.data. uint8 [3]= 0x00;
    outframe.data. uint8 [4]= 0x47;
    outframe.data. uint8 [5]= 0x0A;
    outframe.data. uint8 [6]= 0xDA;
    outframe.data. uint8 [7]= 0x61;
    Can0.sendFrame(outframe); 
    }
    index0x325++;
    if (index0x325>6)
      {index0x325=1;}
  }
}//end Frames 1670ms-------------------------------------------------------------------



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
  {
  CAN_cfg.tx_pin_id = GPIO_NUM_22;
  CAN_cfg.rx_pin_id = GPIO_NUM_23;
  CAN0.begin(500000);// Initialize CAN0 and set baud rate.
  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);   // turn the LED on
  }//end void setup--------------------------------------------------------------------------------------------

void loop()
  {
  Frames20MS();
  Frames200MS();
  Frames1670MS();
  }//end void loop---------------------------------------------------------------------------------------------
