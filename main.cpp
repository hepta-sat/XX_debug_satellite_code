/***
date    : 15th Feb, 2023
made by : Ryusuke Charlie Iwata
note    : Following code is made for debug HEPTA-Sat.
***/
#include "mbed.h"
#include "HEPTA_DEBUG.h"
#include "HEPTA_CDH.h"
#include "HEPTA_EPS.h"
#include "HEPTA_SENSOR.h"
#include "HEPTA_COM.h"
HEPTA_CDH cdh(p5, p6, p7, p8, "sd");
HEPTA_EPS eps(p16,p26);
HEPTA_SENSOR sensor(p17,
                  p28,p27,0x19,0x69,0x13,
                  p13, p14,p25,p24);
HEPTA_COM  com(p9,p10,9600);
RawSerial  sat(USBTX,USBRX,9600);
DigitalOut myleds[]={LED1,LED2,LED3,LED4};
DigitalOut condition(LED1);

/*============== define local variables ==============*/
Timer sattime;
int rcmd = 0,cmdflag = 0; //command variable
char cmd;
float batvol,temp,ax,ay,az,gx,gy,gz,mx,my,mz;
/*====================================================*/

/*======== functions for serial communication ========*/
void end_confirmation();             //confirmation
/*====================================================*/

// main loop
int main() {
    sat.printf(">> Debug satellite code start \r\n");
    sat.printf(">>>> Please hit command which you wanna check from 'a' to 'j'.\r\n");
    sat.printf(">>>> If you wanna check all commands at once , please just hit 'x'\r\n");
    while(1){
        sat.printf(">> -------- Please select following command --------\r\n");
        sat.printf(">>>> %c: Blinking LEDs\r\n", BLINKING_LEDS_CMD);
        sat.printf(">>>> %c: Turn on  3.3V converter\r\n",_3V3_CONVETER_ON_CMD);
        sat.printf(">>>> %c: Turn off 3.3V converter\r\n",_3V3_CONVETER_OFF_CMD);
        sat.printf(">>>> %c: Check timer count \r\n", TIMER_COUNT_CMD);
        sat.printf(">>>> %c: Save letters to SD Card\r\n", SAVE_LETTERS_TO_SD_CMD);
        sat.printf(">>>> %c: Detect the battery voltage\r\n", DETECT_BATTERY_VOL_CMD);
        sat.printf(">>>> %c: Detect the temperature\r\n", DETECT_TEMP_CMD);
        sat.printf(">>>> %c: Detect the 9-axis data\r\n", DETECT_9AXIS_CMD);
        sat.printf(">>>> %c: Detect GPS data as GPGGA format\r\n",DETECT_GPGGA_CMD);
        sat.printf(">>>> %c: Take a snapshot with camera\r\n",TAKE_PIC_CMD);
        sat.printf(">>>> %c: Check uplink and downlink with Xbee\r\n",XBEE_LINK_CMD);
        // sat.printf(">>>> %c: Auto Check Mode from command a to j\r\n",AUTO_CHECK_MODE_CMD);
        sat.printf(">> ------------------------------------------------\r\n");
        eps.turn_on_regulator();
        cmd = sat.getc();
        switch(cmd){
            case BLINKING_LEDS_CMD:
                sat.printf(">> %c: Blinking LEDs MODE Start!\r\n", BLINKING_LEDS_CMD);
                myleds[0] = 0;
                myleds[1] = 0;
                myleds[2] = 0;
                myleds[3] = 0;
                for(int j=0;j<3;j++){
                    sat.printf(">>>> myleds[%d] ON!\r\n", j);
                    myleds[j] = 1;
                    wait_ms(1000);
                }
                end_confirmation();  
                break;
            case _3V3_CONVETER_ON_CMD:
                sat.printf(">> %c: Turn on 3.3V converter\r\n", _3V3_CONVETER_ON_CMD);
                eps.turn_on_regulator();
                sat.printf(">>>> Check the 3.3V converter out line.\r\n");
                sat.printf(">>>> If youve done checking, hit any key. \r\n");
                if(sat.getc()){
                    end_confirmation(); 
                    break;
                }
            case _3V3_CONVETER_OFF_CMD:
                sat.printf(">> %c: Turn off 3.3V converter\r\n", _3V3_CONVETER_OFF_CMD);
                eps.shut_down_regulator();
                sat.printf(">>>> Please check the 3.3V converter out line.\r\n");
                sat.printf(">>>> If youve done checking, hit any key. \r\n");
                if(sat.getc()){
                    end_confirmation(); 
                    break;
                }
            case TIMER_COUNT_CMD:
                sat.printf(">> %c: Check timer count \r\n", TIMER_COUNT_CMD);
                sat.printf(">>>> Timer count start\r\n");
                sattime.start();
                for(int j=0;j<10;j++){
                    sat.printf("Sat Time = %f [s]\r\n",sattime.read());
                    wait_ms(500);
                }
                sattime.stop();
                end_confirmation(); 
                break;
            case SAVE_LETTERS_TO_SD_CMD:
                {
                    sat.printf(">> %c: Save letters to SD Card\r\n", SAVE_LETTERS_TO_SD_CMD);
                    sattime.start();
                    char str[100];
                    mkdir("/sd/mydir", 0777);
                    FILE *fp = fopen("/sd/mydir/debugdata.txt","w");
                    if(fp == NULL) {
                        error("Could not open file for write\r\n");
                    }
                    for(int i = 0; i < 10; i++) {
                        eps.vol(&batvol);
                        fprintf(fp,"HEPTA-Sat DEBUG, Sat time: %f[s]\r\n", sattime.read());
                        condition = 1;
                        wait_ms(1000);
                    }
                    fclose(fp);
                    fp = fopen("/sd/mydir/debugdata.txt","r");
                    for(int i = 0; i < 10; i++) {
                        fgets(str,100,fp);
                        sat.puts(str);
                    }
                    fclose(fp);
                    sattime.stop();
                    end_confirmation();
                    break;
                }
            case DETECT_BATTERY_VOL_CMD:
                sat.printf(">> %c: Detect the battery voltage\r\n", DETECT_BATTERY_VOL_CMD);
                for(int j=0;j<10;j++){
                    eps.vol(&batvol);
                    sat.printf("BatVol = %.2f [V]\r\n",batvol);
                    wait_ms(500);
                }
                end_confirmation(); 
                break;
            case DETECT_TEMP_CMD:
                sat.printf(">> %c: Detect the temperature\r\n", DETECT_TEMP_CMD);
                for(int j=0;j<10;j++){
                    sensor.temp_sense(&temp);
                    sat.printf("Temp = %.2f [degC]\r\n",temp);
                    wait_ms(500);
                }
                end_confirmation(); 
                break;
            case DETECT_9AXIS_CMD:
                sat.printf(">>>> %c: Detect the 9-axis data\r\n", DETECT_9AXIS_CMD);
                for(int j=0;j<10;j++){
                    sensor.sen_acc(&ax,&ay,&az);
                    sensor.sen_gyro(&gx,&gy,&gz);
                    sensor.sen_mag(&mx,&my,&mz);
                    sat.printf("acc : %f,%f,%f\r\n",ax,ay,az);
                    sat.printf("gyro: %f,%f,%f\r\n",gx,gy,gz);
                    sat.printf("mag : %f,%f,%f\r\n\r\n",mx,my,mz);
                    wait_ms(1000);
                }
                end_confirmation(); 
                break;
            case DETECT_GPGGA_CMD:
                {
                    sat.printf(">>>> %c: Detect GPS data as GPGGA format\r\n",DETECT_GPGGA_CMD);
                    sensor.gps_setting();
                    int quality=0,stnum=0,gps_check=0;
                    char ns='A',ew='B',aunit='m';
                    float time=0.0,latitude=0.0,longitude=0.0,hacu=0.0,altitude=0.0;
                    for(int i=1; i<1000; i++) {
                        sensor.gga_sensing(&time, &latitude, &ns, &longitude, &ew, &quality, &stnum, &hacu, &altitude, &aunit, &gps_check);
                        if((gps_check==0)|(gps_check==1)) {
                            sat.printf("GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c\r\n",time,latitude,ns,longitude,ew,quality,stnum,hacu,altitude,aunit);
                        }
                    }
                    end_confirmation(); 
                    break;
                }
            case TAKE_PIC_CMD:
                {
                    sat.printf(">> %c: Take a snapshot with camera\r\n",TAKE_PIC_CMD);
                    FILE *dummy = fopen("/sd/dummy.txt","w");
                    if(dummy == NULL) {
                        error("Could not open file for write\r\n");
                    }
                    fclose(dummy);
                    
                    sat.printf("Camera Snapshot Mode\r\n");
                    sat.printf("Hit Any Key To Take Picture\r\n");
                    while(!sat.readable()) {}
                    sensor.Sync();
                    sensor.initialize(HeptaCamera_GPS::Baud115200, HeptaCamera_GPS::JpegResolution320x240);
                    sensor.test_jpeg_snapshot_picture("/sd/debug.jpg");
                    end_confirmation(); 
                    break;
                }
            case XBEE_LINK_CMD:
                sat.printf(">> %c: Check uplink and downlink with Xbee\r\n",XBEE_LINK_CMD);
                while(1){
                    com.xbee_receive(&rcmd,&cmdflag);
                    sat.printf(">>>> Hit 'a' as an uplink command\r\n");
                    if (cmdflag == 1) {
                        if (rcmd == 'a') {
                            sattime.start();
                            sat.printf(">>>> Command Get, rcmd: %d\r\n",rcmd);
                            com.printf(">>>> HEPTA Uplink OK, rcmd: %d\r\n",rcmd);
                            for(int j=0;j<10;j++){
                                com.printf(">>>> Sat Time = %f [s]\r\n",sattime.read());
                                wait_ms(500);
                            }
                            end_confirmation(); 
                            break;
                        }
                    }
                    com.initialize();
                    wait(1);
                }
                break;
            case AUTO_CHECK_MODE_CMD:
                break;

            default:
                sat.printf(">> Hit any key from 'a' to 'k' or 'x' instead..\r\n");
                break;
        } //swith end
    } //while(1) end
}


/*====================================================*/
// define functions
void end_confirmation(){
    for(int j=0;j<5;j++){
        myleds[0] = 1;
        myleds[1] = 1;
        myleds[2] = 1;
        myleds[3] = 1;
        wait_ms(200);
        myleds[0] = 0;
        myleds[1] = 0;
        myleds[2] = 0;
        myleds[3] = 0;
        wait_ms(200);
    }
    sat.printf(">>>> Finish!! \r\n\r\n");
}
/*====================================================*/