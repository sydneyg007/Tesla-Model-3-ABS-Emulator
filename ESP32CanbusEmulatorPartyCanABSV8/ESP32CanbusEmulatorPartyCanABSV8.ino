//V3 modifying 0x145 byte 7,8 and checksum. 0x185 Adding brake torque (was zero), fixed 0x38C. chande 0x38D
//ver=4; deleted start up frames
//ver=5; added led on indicator to pin 32
//ver=6;  from wake/routine3/P3C6_Reference.csv
          //started 0x175 couner at 0x10 and checksum at 0x86,
          //changed 0x38C byte 2 to 00 and byte 3 to 7E also adjust counter and CRC,
          //modified 0x38D changed bytes 5,6 and modified crc
          //0x11D Changed bytes to all zeros and modified checksum to suit
          //0x145 modified bytes so no alerts?
          //0x155 first 2 frame different (initialise),also changed main frames
          //0x185 changed (first frames start at all zero - may need to modify when brakes applied
          //0x38B Changed to all zeros and added different first frame, modified crc to suit
          //0x385 added first 4 (initialisation?) frames
//ver=7; 0x11D added systemTest0x11D to turn off at 2000ms
//ver=8; 0x38B Brake torque - increase byte 3 to show brakes where applying torque
          

//CanTX pin - gpio 22
//CanRX pin - gpio 23

#include <esp32_can.h>

//Timers-------------------------------------------------------------------------------------
unsigned long timer10ms = 0;        // will store last time was updated
unsigned long timer20ms = 0;        // will store last time was updated

byte counter0x175 = 0x10; //(0x00 to 0xF0, increment by 0x10)
byte checksum0x175 = 0x86; //byte 6 +0x76

byte counter0x38C = 0x01; //(0x01 to 0x0F, increment by + 1)
byte crc0x38C = 0x81; //use look up table

byte counter0x38D = 0x01; //(0x00 to 0xF0, increment by + 1)
byte crc0x38D = 0x8B; //use look up table

byte counter0x105 = 0x10; //(0x00 to 0xF0, increment by 0x10 (16))
byte checksum0x105 = 0x52; //(0C to FC) increments by 0x10(16) ,Resets to 0x00 when counter =0x 4C

byte systemTest0x11D = 0x00; //0x00=System Testing 0x80=Test OK
byte counter0x11D = 0x01; //(0x00 to 0x0F, increment by + 1)
byte checksum0x11D = 0x1F; // counter0x11D +0x9E (0x9E to 0xAD)

byte counter0x145 = 0x41; //increment by + 1
byte checksum0x145 = 0xFC; // increment by + 1

byte frameCount0x155 = 0; 
byte counter0x155 = 0x30; //(0x00 to 0xF0, increment by 0x10 (16))
byte checksum0x155 = 0x86; //(0x03 to F3) counter0x155+0x10;

byte counter0x185 = 0x14; // increment by 0x10 )
byte checksum0x185 = 0x9A; //increment by 0x10  )

byte frameCount0x385 = 0;
byte counter0x385 = 0x10; //(0x00 to 0xF0, increment by 0x10 (16))
byte checksum0x385 = 0x94; //(0x0A to 0xFA) (increment by 0x10 (16)) )

byte frameCount0x38B = 0; 
byte counter0x38B = 0x42; //increment by + 1
byte crc0x38B = 0x92; //use look up table

CAN_FRAME outframe;

void Frames10MS()
{
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

//0x38C
        outframe.id = 0x38C;            // Set our transmission address ID
        outframe.length = 4;            // Data payload 8 bytes
        outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
        outframe.rtr=0;                 //No request
        outframe.data. uint8 [0]= crc0x38C;
        outframe.data. uint8 [1]= counter0x38C;
        outframe.data. uint8 [2]= 0x00;
        outframe.data. uint8 [3]= 0x7E;
        Can0.sendFrame(outframe); 
        //Byte1 calc
        counter0x38C ++;
        if (counter0x38C > 0x0F)
        {counter0x38C = 0x00;}
        //Byte0 calc
        //Byte0 Look up table
        switch (counter0x38C) {
            case 0x00: crc0x38C =0x0E; break; 
            case 0x01: crc0x38C =0x81; break; 
            case 0x02: crc0x38C =0x0D; break; 
            case 0x03: crc0x38C =0x82; break;
            case 0x04: crc0x38C =0x08; break;
            case 0x05: crc0x38C =0x87; break; 
            case 0x06: crc0x38C =0x0B; break;
            case 0x07: crc0x38C =0x84; break;
            case 0x08: crc0x38C =0x02; break;
            case 0x09: crc0x38C =0x8D; break;
            case 0x0A: crc0x38C =0x01; break;
            case 0x0B: crc0x38C =0x8E; break;
            case 0x0C: crc0x38C =0x04; break;
            case 0x0D: crc0x38C =0x8B; break;
            case 0x0E: crc0x38C =0x07; break;
            case 0x0F: crc0x38C =0x88; break;     
          }

//0x38D
        outframe.id = 0x38D;            // Set our transmission address ID
        outframe.length = 8;            // Data payload 8 bytes
        outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
        outframe.rtr=0;                 //No request
        outframe.data. uint8 [0]= crc0x38D;
        outframe.data. uint8 [1]= counter0x38D;
        outframe.data. uint8 [2]= 0x00;
        outframe.data. uint8 [3]= 0x00;
        outframe.data. uint8 [4]= 0x00;
        outframe.data. uint8 [5]= 0x00;
        outframe.data. uint8 [6]= 0x10;
        outframe.data. uint8 [7]= 0x4C;
        Can0.sendFrame(outframe); 
          //Byte1 calc
          counter0x38D ++;
          if (counter0x38D >0x0F)
          {counter0x38D =0x00;}
              //Byte0  calc
              //Byte 0 Look up table
              switch (counter0x38D) {
                  case 0x00: crc0x38D =0xCD; break; 
                  case 0x01: crc0x38D =0x8B; break; 
                  case 0x02: crc0x38D =0x41; break; 
                  case 0x03: crc0x38D =0x07; break;
                  case 0x04: crc0x38D =0xC8; break; 
                  case 0x05: crc0x38D =0x8E; break; 
                  case 0x06: crc0x38D =0x44; break;
                  case 0x07: crc0x38D =0x02; break;
                  case 0x08: crc0x38D =0xC7; break;
                  case 0x09: crc0x38D =0x81; break;
                  case 0x0A: crc0x38D =0x4B; break;
                  case 0x0B: crc0x38D =0x0D; break;
                  case 0x0C: crc0x38D =0xC2; break;
                  case 0x0D: crc0x38D =0x84; break;
                  case 0x0E: crc0x38D =0x4E; break;
                  case 0x0F: crc0x38D =0x08; break;     
                }

                
}//end Frames 10ms-------------------------------------------------------------------


void Frames20MS()
{
//0x105
        outframe.id = 0x105;            // Set our transmission address ID
        outframe.length = 8;            // Data payload 8 bytes
        outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
        outframe.rtr=0;                 //No request
        outframe.data. uint8 [0]= 0x00;
        outframe.data. uint8 [1]= 0x00;
        outframe.data. uint8 [2]= 0x00;
        outframe.data. uint8 [3]= 0x00;
        outframe.data. uint8 [4]= 0x30;
        outframe.data. uint8 [5]= 0x0C;
        outframe.data. uint8 [6]=counter0x105;
        outframe.data. uint8 [7]=checksum0x105;
        Can0.sendFrame(outframe); 
        //Byte 6 calc
        counter0x105=counter0x105+0x10;
        if (counter0x105>0xF0)
          {counter0x105=0x00;}
        //Byte 7 calc
        checksum0x105=checksum0x105+0x10;
        if (checksum0x105>0xF2)
          {checksum0x105=0x02;}

//0x11D
        outframe.id = 0x11D;            // Set our transmission address ID
        outframe.length = 8;            // Data payload 8 bytes
        outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
        outframe.rtr=0;                 //No request
        outframe.data. uint8 [0]= checksum0x11D;
        outframe.data. uint8 [1]= counter0x11D;
        outframe.data. uint8 [2]= 0x00;
        outframe.data. uint8 [3]= 0x00;
        outframe.data. uint8 [4]= 0x00;
        outframe.data. uint8 [5]= 0x00;
        outframe.data. uint8 [6]= systemTest0x11D;
        outframe.data. uint8 [7]= 0x00;
        Can0.sendFrame(outframe); 
        //Byte1 calc
        counter0x11D ++;
        if (counter0x11D >0x0F)
        {counter0x11D =0x00;}
        //Byte0  calc
        checksum0x11D=0x01+0x1D+systemTest0x11D+counter0x11D; //id numbers added together + all other numbers added
        if (millis()>2000){systemTest0x11D=0x80;}

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

            //0x385 
            if (frameCount0x385<4)
              {outframe.id = 0x385;            // Set our transmission address ID
              outframe.length = 8;            // Data payload 8 bytes
              outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
              outframe.rtr=0;                 //No request
              outframe.data. uint8 [0]= 0xFF;
              outframe.data. uint8 [1]= 0xFF;
              outframe.data. uint8 [2]= 0xFF;
              outframe.data. uint8 [3]= 0xFF;
              outframe.data. uint8 [4]= 0x00;
              outframe.data. uint8 [5]= 0x00;
              outframe.data. uint8 [6]= counter0x385;
              outframe.data. uint8 [7]= checksum0x385;
              Can0.sendFrame(outframe); 
              counter0x385 = counter0x385 + 0x10;
              checksum0x385=checksum0x385+0x10;
              if (checksum0x385>0xC4)
                {checksum0x385=0xAA;} 
              frameCount0x385++;
              }

            if (frameCount0x385>=4)
              {outframe.id = 0x385;            // Set our transmission address ID
              outframe.length = 8;            // Data payload 8 bytes
              outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
              outframe.rtr=0;                 //No request
              outframe.data. uint8 [0]= 0x71;
              outframe.data. uint8 [1]= 0x6F;
              outframe.data. uint8 [2]= 0xF4;
              outframe.data. uint8 [3]= 0xFE;
              outframe.data. uint8 [4]= 0x00;
              outframe.data. uint8 [5]= 0x00;
              outframe.data. uint8 [6]= counter0x385;
              outframe.data. uint8 [7]= checksum0x385;
              Can0.sendFrame(outframe); 
              counter0x385 = counter0x385 + 0x10;
              if (counter0x385 > 0xF0)
                {counter0x385 = 0x00;}
              checksum0x385 = checksum0x385 + 0x10;
              if (checksum0x385 > 0xFA)
                {checksum0x385 = 0x0A;}
              }  

        //0x38B brake torque
        outframe.id = 0x38B;            // Set our transmission address ID
        outframe.length = 4;            // Data payload 8 bytes
        outframe.extended = 0;          // Extended addresses - 0=11-bit 1=29bit
        outframe.rtr=0;                 //No request
        outframe.data. uint8 [0]= crc0x38B;
        outframe.data. uint8 [1]= counter0x38B;
        outframe.data. uint8 [2]= 0x00;
        outframe.data. uint8 [3]= 0x1E; //brake torque amount
        Can0.sendFrame(outframe); 
        //Byte1 calc
        counter0x38B ++;
        if (counter0x38B >0x4F)
        {counter0x38B = 0x40;}
        //Byte0 calc
        //Byte0 Look up table
        switch (counter0x38B) {
            case 0x40: crc0x38B =0xFA; break; 
            case 0x41: crc0x38B =0x75; break; 
            case 0x42: crc0x38B =0xF9; break; 
            case 0x43: crc0x38B =0x76; break;
            case 0x44: crc0x38B =0xFC; break;
            case 0x45: crc0x38B =0x73; break; 
            case 0x46: crc0x38B =0xFF; break;
            case 0x47: crc0x38B =0x70; break;
            case 0x48: crc0x38B =0xF6; break;
            case 0x49: crc0x38B =0x79; break;
            case 0x4A: crc0x38B =0xF5; break;
            case 0x4B: crc0x38B =0x7A; break;
            case 0x4C: crc0x38B =0xF0; break;
            case 0x4D: crc0x38B =0x7F; break;
            case 0x4E: crc0x38B =0xF3; break;
            case 0x4F: crc0x38B =0x7C; break;     
          }
                
}//end Frames 20ms-------------------------------------------------------------------



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{

CAN_cfg.tx_pin_id = GPIO_NUM_22;
CAN_cfg.rx_pin_id = GPIO_NUM_23;
CAN0.begin(500000);// Initialize CAN0 and set baud rate.
pinMode(32, OUTPUT);
digitalWrite(32, HIGH);   // turn the LED on
}//end void setup--------------------------------------------------------------------------------------------

void loop(){
  
if (millis() >= timer10ms + 10) {
timer10ms=millis();
Frames10MS();
}
if (millis() >= timer20ms + 20) {
timer20ms=millis();
Frames20MS();
}

}//end void loop---------------------------------------------------------------------------------------------
