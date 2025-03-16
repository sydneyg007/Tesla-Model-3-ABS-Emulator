//v18 modifying from basic can bridge - seems to be successfull in modifying 0x257 to emulate speeds in 0x155 and 0x175
//v19 All frames now go through from vehicle side to esp side, can be used for both party and chassis cans 
//v20 need to fix errors occuring when foot on brake - 0x155 ESP_standstillStatus in non error file reads initially as rolling before standstill a little after foot on brake.
      //ESP wheel rotations go from unknown to standstill (and dont align excactly with standStillStatus)
//V21 removed send 0x155 and 0x175 only if moving - maybe use Di-axlespeed (0x108) instead of 0x257 (but make sure doesn't go negative)
//V22 useing Di_axlespeed (0x108) instead of 0x257
//V23 adding 0x155 wheel tick increments
//V24 actually useing Di_axlespeed (0x108) instead of 0x257 (didn't implement before - oops)
//V25 Tidy up
//v26 going back to 0x257 for forward backward sigs
//v27 clean up
//v28 going back to start from basic can bridge
//V29 implementing new code from ESP32Tcan485frameModtesting, succesfully emulating 0x155 and 0x175 (still haven't tackled 0x11D)
//v30 scaling ESP_0x155_Byte0,1,2,3 from di_axlespeed (wheelrotation clicks) Divide di_axlespeed by ~300 to get the correct click increment
    //note: 0x108 happens every 10ms so need to halve wheel click increments (i.e. /600 will happen twice between 0x155 frames so will equal /300 each time)
//v31 adding more dead zone for standstill status (axle speed 100 up)
//v33 modifying V31 and reducing deadzone to 10
//v34 Need to add more frames to account for wheel speed sensors now being unplugged - ADDING 0x145 frames - Not working... brake torque target and ebr status remain at zero...??? Also 11d stays at zero??
      //0x11D (party only) starts at 0 then goes to 0x80 when foot on brake
      //0x105 (party only) byte 4 and 5 seem to relate to brake pressures - can probably let go though unmodified
      //0x145 (both) espstatus has error messages (both cans)
      //0x155 (both) espwheel rotations emulate from DI_axle speed - be carefull with standstill status as it initialises as rolling
      //0x175 (both) espwheelspeeds emulate from Di_axlespeed
      //0x185 (both) Esp_brakeTorque  - let pass through unmodified
      //0x325 (chassis only) FC_alertMatrix  - can probably let go though unmodified
//v35 Need to fic 0x155 wheel clicks (not working going forwards, only work going backwards!) using simple byte++ works in both directions till about 15kph
//v36 implementing 0x155 clicks more elegantly... every 1000 di axle speed raw increment by 1 - working better but still errors out occasionally
//v37


//PartyCan: 0x105(261), 0x11D(285), 0x145(325), 0x155(341), 0x175(373), 0x185(389),            0x385(901), 0x38B(907), 0x38C(908), 0x38D(909), 
//ChassisCan:                       0x145(325), 0x155(341), 0x175(373), 0x185(389), 0x325(805),                                               0x3D5(981) , 0x505(don't think this one applies)
//wakeup frame id = 0x39D (IBST Status)

#include "src/due_can.h" //required library

//0x108 (264) Di_torque               
int DI_axleSpeedraw;  // 40|16@1- (0.1,0) [0|0] "RPM"
int D_esp_wheelspeedraw;   

//0x145 (325) ESP_status
byte ESP_driverBrakeApply = 2; //29|2@1+ (1,0) [0|0] "", 0 "NotInit_orOff" 1 "Not_Applied" 2 "Driver_applying_brakes" 3 "Faulty_SNA";
byte ESP_brakeApply = 1; //31|1@1+ (1,0) [0|0] "", 0 "INACTIVE" 1 "ACTIVE";
byte ESP_statusCounter = 0x01; //8|4@1+ (1,0) [0|0] ""
byte ESP_statusChecksum = 0x0E; //(byte zero) 0|8@1+ (1,0) [0|0] "" (sum of bytes 1-7 + 0x45 + 0x01)
//byte ESP_0x145_Byte0 = 0x0E; //checksum
byte ESP_0x145_Byte1 = 0x01;
byte ESP_0x145_Byte2 = 0x00;
byte ESP_0x145_Byte3 = 0xCF;
byte ESP_0x145_Byte4 = 0xA4;
byte ESP_0x145_Byte5 = 0x00; //stays at 0x00 (no definition)
byte ESP_0x145_Byte6 = 0x00;
byte ESP_0x145_Byte7 = 0x00;

//0x155 ESP_wheelRotation variables
byte ESP_wheelRotationQF = 0x00; //1=valid, 0=invalid
byte ESP_standstillStatus = 0x00; //0=rolling (initialising), 1=standstill
unsigned int ESP_vehicleSpeed = 511; // is 10 bits so needs to be an int (((ESP_0x155_Byte6 & B00001111)<<2)+((ESP_0x155_Byte5 & B11111100)>> 2))*.5; first frame 511kph (initialising?)
unsigned int ESP_vehicleSpeedraw = 1022; //first frame 511kph (initialising?) raw is x2 (1022)
byte ESP_wheelRotationCounter = 0x00;
uint8_t ESP_wheelRotationChecksum = 0x6C;
byte ESP_wheelAngle_0x155 = 0xFF; //initialise as 0xFF (initialising)
byte ESP_0x155_Byte4 = 0xFF; //initialise as 0xFF (unknown - initialising) ESP_wheelRotationRRRLFRFL - each wheel 2 bits x4 takes whole byte (0xAA=all standstill, 0x00=all forward, 0xFF=all unknown, 0x55=all backwards)
byte ESP_0x155_Byte5 = 0xFC; //initialise as 0xFC (initialising)
byte ESP_0x155_Byte6 = 0x1F; //initialise as 0x1F ESP_wheelRotationCounter=1, ESP_vehicleSpeed=511kph
byte ESP_0x155_Byte7 = 0x6C; //initialise as 0x6C ESP_wheelRotationChecksum

byte ESP_wheelAngle_0x155_increment = 1;

//0x175 (373) ESP_wheelSpeed variables  ESP_wheelSpeedRR : 39|13@1+ (0.04,0) [0|327.64] "RPM"
byte ESP_0x175_Byte0 = 0x00; //initialise as 0x00=ESP_wheelspeed: 0 ESP_wheelSpeedFL : 0|13
byte ESP_0x175_Byte1 = 0x00; //initialise as 0x00=ESP_wheelspeed: 0 ESP_wheelSpeedFR : 13|13, ESP_wheelSpeedFL : 0|13
byte ESP_0x175_Byte2 = 0x00; //initialise as 0x00=ESP_wheelspeed: 0 ESP_wheelSpeedFR : 13|13
byte ESP_0x175_Byte3 = 0x00; //initialise as 0x00=ESP_wheelspeed: 0 ESP_wheelSpeedRL : 26|13, ESP_wheelSpeedFR : 13|13
byte ESP_0x175_Byte4 = 0x00; //initialise as 0x00=ESP_wheelspeed: 0 ESP_wheelSpeedRR : 39|13
byte ESP_0x175_Byte5 = 0x00; //initialise as 0x00=                  ESP_wheelSpeedRR : 39|13
byte ESP_0x175_Byte6 = 0x10; //initialise as 0x10 ESP_wheelSpeedCounter : 52|4, ESP_wheelSpeedRR : 39|13
byte ESP_0x175_Byte7 = 0x86; //initialise as 0x86 ESP_wheelSpeedChecksum : 56|8
byte ESP_wheelspeedCounter = 0x06;
uint8_t ESP_wheelspeedChecksum = 0xD6;

void setup()
{
Can0.begin(CAN_BPS_500K); //vehicle side 
Can1.begin(CAN_BPS_500K); //esp side

int filter;
for (int filter = 3; filter < 7; filter++) {
  Can0.setRXFilter(filter, 0, 0, false);
  Can1.setRXFilter(filter, 0, 0, false);
  }//end setup filters (standard frames)
}//end void setup

void loop(){
CAN_FRAME incoming;

//send frame from vehicle to esp  
if (Can0.available() > 0) {//if vehicle side can available
	Can0.read(incoming);

    if (incoming.id == 0x108){//vehicle side 0x108 DI_torque
      DI_axleSpeedraw = incoming.data.byte[5] + (incoming.data.byte[6]<<8); //*.1

      //ESP_wheelRotationRRRLFRFL direction unknown
      //if (((DI_axleSpeedraw > 3) && (DI_axleSpeedraw < 100)) || ((DI_axleSpeedraw > 65505) && (DI_axleSpeedraw < 65533))){ESP_0x155_Byte4 = 0xFF;} 
      //if(ESP_wheelAngle_0x155==0xFF){ESP_wheelAngle_0x155=0x00;}
      //going forwards
      if ((DI_axleSpeedraw >= 5) && (DI_axleSpeedraw < 32767)){D_esp_wheelspeedraw = DI_axleSpeedraw/3.2; ESP_vehicleSpeedraw = DI_axleSpeedraw/40; ESP_standstillStatus=0x00; ESP_0x155_Byte4 = 0x00;
        
        //old code
        if (ESP_wheelAngle_0x155 == 0xFF){ESP_wheelAngle_0x155=0x00;} else {ESP_wheelAngle_0x155 ++;} //ESP_wheelAngleFL

        //ESP_wheelAngle_0x155_increment = constrain(ESP_wheelAngle_0x155_increment, 1, 255); 

        //ESP_wheelAngle_0x155_increment = map(DI_axleSpeedraw, 0, 2700, 0, 8);  //(map(value, fromLow, fromHigh, toLow, toHigh)

        //ESP_wheelAngle_0x155 = ESP_wheelAngle_0x155 + ESP_wheelAngle_0x155_increment;
        
        //if (ESP_0x155_Byte0 == 0xFF){ESP_0x155_Byte0=0x00;} else {ESP_0x155_Byte0 = ESP_0x155_Byte0 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleFL
        //if (ESP_0x155_Byte1 == 0xFF){ESP_0x155_Byte1=0x00;} else {ESP_0x155_Byte1 = ESP_0x155_Byte1 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleFR
        //if (ESP_0x155_Byte2 == 0xFF){ESP_0x155_Byte2=0x00;} else {ESP_0x155_Byte2 = ESP_0x155_Byte2 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleRL
        //if (ESP_0x155_Byte3 == 0xFF){ESP_0x155_Byte3=0x00;} else {ESP_0x155_Byte3 = ESP_0x155_Byte3 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleRR   
        }

      //going backwards
      if ((DI_axleSpeedraw < 65530) && (DI_axleSpeedraw > 32769)){D_esp_wheelspeedraw = (65535 - DI_axleSpeedraw)/3.2; ESP_vehicleSpeedraw = (65535 - DI_axleSpeedraw)/40; ESP_standstillStatus=0x00; ESP_0x155_Byte4 = 0x55;
        
        //old code
        if (ESP_wheelAngle_0x155 == 0xFF){ESP_wheelAngle_0x155=0x00;} else {ESP_wheelAngle_0x155 ++;} //ESP_wheelAngleFL

        //ESP_wheelAngle_0x155_increment = constrain(ESP_wheelAngle_0x155_increment, 1, 255);  
        
        //ESP_wheelAngle_0x155_increment = map(65535 - DI_axleSpeedraw, 0, 2700, 0, 8);  //(map(value, fromLow, fromHigh, toLow, toHigh)

        //ESP_wheelAngle_0x155 = ESP_wheelAngle_0x155 + ESP_wheelAngle_0x155_increment;
        
        //ESP_0x155_Byte0 = ESP_0x155_Byte0 + ((65535 - DI_axleSpeedraw)/600);
        //ESP_0x155_Byte1 = ESP_0x155_Byte1 + ((65535 - DI_axleSpeedraw)/600);
        //ESP_0x155_Byte2 = ESP_0x155_Byte2 + ((65535 - DI_axleSpeedraw)/600);
        //ESP_0x155_Byte3 = ESP_0x155_Byte3 + ((65535 - DI_axleSpeedraw)/600);
        
        //if (ESP_0x155_Byte0 == 0xFF){ESP_0x155_Byte0=0x00;} else {ESP_0x155_Byte0 = ESP_0x155_Byte0 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleFL
        //if (ESP_0x155_Byte1 == 0xFF){ESP_0x155_Byte1=0x00;} else {ESP_0x155_Byte1 = ESP_0x155_Byte1 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleFR
        //if (ESP_0x155_Byte2 == 0xFF){ESP_0x155_Byte2=0x00;} else {ESP_0x155_Byte2 = ESP_0x155_Byte2 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleRL
        //if (ESP_0x155_Byte3 == 0xFF){ESP_0x155_Byte3=0x00;} else {ESP_0x155_Byte3 = ESP_0x155_Byte3 + (DI_axleSpeedraw/600);}  //ESP_wheelAngleRR    
        } 

      //standstill
      if ((DI_axleSpeedraw < 5) || (DI_axleSpeedraw > 65530)){D_esp_wheelspeedraw = 0; ESP_vehicleSpeedraw = 0; ESP_standstillStatus=0x01; ESP_0x155_Byte4 = 0xAA;} 

      //calculate 0x155 ESP_vehicleSpeed (ESP_vehicleSpeed take up 10 bits (0x000003ff))
      ESP_0x155_Byte5 = (((ESP_vehicleSpeedraw) & 0x0000003f)<<2) + (ESP_standstillStatus << 1) + ESP_wheelRotationQF; 
      ESP_0x155_Byte6 = (ESP_wheelRotationCounter << 4) + (((ESP_vehicleSpeedraw) & 0x000003C0) >> 6);
      ESP_wheelRotationChecksum = ESP_wheelAngle_0x155 + ESP_wheelAngle_0x155 + ESP_wheelAngle_0x155 + ESP_wheelAngle_0x155 + ESP_0x155_Byte4 + ESP_0x155_Byte5 + ESP_0x155_Byte6 + 0x55 + 1;//sum of bytes (less checksum byte) plus id rh 2 digits plus id first digit (so 0x155 = 0x55 + 0x01)

      //calculate 0x175 esp_wheelspeeds (wheelspeeds take up 13 bits (0x00001fff))
      ESP_0x175_Byte0 = ((D_esp_wheelspeedraw) & 0x000000ff); //using hex "anding" because "esp_wheelspeed" is an integer
      ESP_0x175_Byte1 = (((D_esp_wheelspeedraw) & 0x00001fff)>>8)+(((D_esp_wheelspeedraw) & 0x00000007)<<5);
      ESP_0x175_Byte2 = (((D_esp_wheelspeedraw) & 0x000007ff)>>3);
      ESP_0x175_Byte3 = (((D_esp_wheelspeedraw) & 0x00001fff)>>11)+(((D_esp_wheelspeedraw) & 0x0000003f)<<2);
      ESP_0x175_Byte4 = (((D_esp_wheelspeedraw) & 0x00001fc0)>>6)+(((D_esp_wheelspeedraw) & 0x000000ff)<<7);
      ESP_0x175_Byte5 = (((D_esp_wheelspeedraw) & 0x00001fff)>>1);
      ESP_0x175_Byte6 = (((D_esp_wheelspeedraw) & 0x00001fff)>>9)+((ESP_wheelspeedCounter)<<4);
      ESP_0x175_Byte7 = ESP_0x175_Byte0 + ESP_0x175_Byte1 + ESP_0x175_Byte2 + ESP_0x175_Byte3 + ESP_0x175_Byte4 + ESP_0x175_Byte5 + ESP_0x175_Byte6 + 0x75 + 0x01;//sum of bytes (excluding checksum byte) plus id rh 2 digits plus id first digit (so 0x175 = 0x75 + 0x01) (will only use RH 8 bits)
      
      ESP_wheelRotationQF=0x01; //set to valid  
      }//end 0x108 frame
  
  Can1.sendFrame(incoming);//send to esp can 
}//end if vehicle side can available

//send frames from esp to vehicle 
if (Can1.available() > 0) {//if esp can available
	Can1.read(incoming);

      if (incoming.id == 0x145){//0x145 ESP_status
      ESP_driverBrakeApply = (incoming.data.uint8[3] & B01100000)>>5;  //29|2@1+ (1,0) [0|0] "" 0 "NotInit_orOff" 1 "Not_Applied" 2 "Driver_applying_brakes" 3 "Faulty_SNA";
      ESP_brakeApply = (incoming.data.uint8[3] & B10000000)>>7; //31|1@1+ (1,0) [0|0] "", 0 "INACTIVE" 1 "ACTIVE";
      ESP_0x145_Byte1 = ESP_statusCounter + 0x40; 
      ESP_0x145_Byte3 = (ESP_driverBrakeApply << 5) + (ESP_brakeApply <<7) + 0x0f;
      ESP_0x145_Byte6 = incoming.data.uint8[6];
      ESP_0x145_Byte7 = incoming.data.uint8[7];
      ESP_statusChecksum = ESP_0x145_Byte1 + ESP_0x145_Byte2 + ESP_0x145_Byte3 + ESP_0x145_Byte4 + ESP_0x145_Byte5 + ESP_0x145_Byte6 + ESP_0x145_Byte7 + 0x45 + 1;//sum of bytes (less checksum byte) plus id rh 2 digits plus id first digit (so 0x155 = 0x55 + 0x01)
      incoming.data.uint8[0] = ESP_statusChecksum; // 0|8@1+ (1,0) [0|0] "" (sum of bytes 1-7 + 0x45 + 0x01)
      incoming.data.uint8[1] = ESP_0x145_Byte1; //ESP_statusCounter: 8|4@1+ (1,0) [0|0] "", ESP_espModeActive: 12|2@1+ (1,0) [0|0] ""(always 0), ESP_stabilityControlSts2: 14|2@1+ (1,0) [0|0] "" (init at 0 then 1 for on (0x40))
      incoming.data.uint8[2] = ESP_0x145_Byte2; //(leave at 0x00) ESP_ebdFaultLamp: 16|1@1+ (1,0) [0|0] "", ESP_absFaultLamp: 17|1@1+ (1,0) [0|0] "", ESP_espFaultLamp: 18|1@1+ (1,0) [0|0] "", ESP_hydraulicBoostEnabled: 19|1@1+ (1,0) [0|0] ""(always 0), ESP_espLampFlash: 20|1@1+ (1,0) [0|0] ""(always 0), ESP_brakeLamp: 21|1@1+ (1,0) [0|0] ""(always 0), ESP_absBrakeEvent2: 22|2@1+ (1,0) [0|0] ""(always 0),  
      incoming.data.uint8[3] = ESP_0x145_Byte3; //ESP_longitudinalAccelQF: 24|1@1+ (1,0) [0|0] ""(1), ESP_lateralAccelQF: 25|1@1+ (1,0) [0|0] ""(1), ESP_yawRateQF: 26|1@1+ (1,0) [0|0] ""(1), ESP_steeringAngleQF: 27|1@1+ (1,0) [0|0] "" (1), ESP_brakeDiscWipingActive: 28|1@1+ (1,0) [0|0] ""(0), ESP_driverBrakeApply: 29|2@1+ (1,0) [0|0] ""(read state), ESP_brakeApply: 31|1@1+ (1,0) [0|0] "" (read state)
      incoming.data.uint8[4] = ESP_0x145_Byte4; //(leave at 0xA4) ESP_cdpStatus: 34|2@1+ (1,0) [0|0] ""(1), ESP_ptcTargetState: 36|2@1+ (1,0) [0|0] ""(2), ESP_btcTargetState: 38|2@1+ (1,0) [0|0] ""(2)
      incoming.data.uint8[5] = ESP_0x145_Byte5; //stays at 0x00 (no definition)
      incoming.data.uint8[6] = ESP_0x145_Byte6; // (let this one pass through?) ESP_ebrStandstillSkid: 48|1@1+ (1,0) [0|0] ""(0), ESP_ebrStatus: 49|2@1+ (1,0) [0|0] ""(activates when brake torque target not 0), ESP_brakeTorqueTarget: 51|13@1+ (2,0) [0|0] "Nm"
      incoming.data.uint8[7] = ESP_0x145_Byte7; // (let this one pass through) ESP_brakeTorqueTarget: 51|13@1+ (2,0) [0|0] "Nm"
      ESP_statusCounter++;
      if (ESP_statusCounter>0x0F){ESP_statusCounter=0;}
      }//end 0x145 ESP_wheelRotation 

    if (incoming.id == 0x155){//0x155 ESP_wheelRotation
      incoming.data.uint8[0] = ESP_wheelAngle_0x155; //ESP_wheelAngleFL
      incoming.data.uint8[1] = ESP_wheelAngle_0x155; //ESP_wheelAngleFR
      incoming.data.uint8[2] = ESP_wheelAngle_0x155; //ESP_wheelAngleRL
      incoming.data.uint8[3] = ESP_wheelAngle_0x155; //ESP_wheelAngleRR
      incoming.data.uint8[4] = ESP_0x155_Byte4; //ESP_wheelRotation
      incoming.data.uint8[5] = ESP_0x155_Byte5; //ESP_wheelRotationQF 40|1, ESP_standstillStatus 41|1, ESP_vehicleSpeed 42|10
      incoming.data.uint8[6] = ESP_0x155_Byte6; //ESP_wheelRotationCounter 52|4, ESP_vehicleSpeed 42|10
      incoming.data.uint8[7] = ESP_wheelRotationChecksum; //ESP_wheelRotationChecksum
      ESP_wheelRotationCounter++;
      if (ESP_wheelRotationCounter>0x0F){ESP_wheelRotationCounter=0x00;}
      }//end 0x155 ESP_wheelRotation 

    if (incoming.id == 0x175){//0x175 ESP_wheelSpeed
      incoming.data.uint8[0] = ESP_0x175_Byte0; 
      incoming.data.uint8[1] = ESP_0x175_Byte1; 
      incoming.data.uint8[2] = ESP_0x175_Byte2; 
      incoming.data.uint8[3] = ESP_0x175_Byte3; 
      incoming.data.uint8[4] = ESP_0x175_Byte4; 
      incoming.data.uint8[5] = ESP_0x175_Byte5; 
      incoming.data.uint8[6] = ESP_0x175_Byte6; 
      incoming.data.uint8[7] = ESP_0x175_Byte7; 
      ESP_wheelspeedCounter++;
      if (ESP_wheelspeedCounter>0x0F){ESP_wheelspeedCounter=0x00;}
      }//end 0x175 ESP_wheelSpeed

  Can0.sendFrame(incoming);//send esp canbus frames to vehicle side canbus
  }//end if esp can available

}//end loop


