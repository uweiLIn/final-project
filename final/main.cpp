#include"mbed.h"
#include "bbcar.h"


Ticker servo_ticker;
Ticker encoder_ticker;
PwmOut pin5(D5), pin6(D6);
BufferedSerial xbee(D10, D9);
DigitalIn encoder(D11);
volatile int steps;
volatile int last;
DigitalInOut ping(D12);
Timer t;

BufferedSerial pc(USBTX,USBRX); //tx,rx
BufferedSerial uart(D1,D0); //tx,rx
BBCar car(pin5, pin6, servo_ticker);

Thread thread1, thread2, thread3;


char value;
char read1;
int angle;

int output1[4] = {0};
int output2[4] = {0};
int store1 = 1;
int store2 = 1;
int line = 0;
int tag = 0;
int  circle = 0;
float val = 0;

float dis, dis_xbee;

float factor;
char buffer[10];


void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}

void line_detect(){
    char read;
    int mode = 1;
    while(line) {
        mode = 1;
        if(uart.readable()){
            char recv[1];
            uart.read(recv, sizeof(recv));
            read = *recv;
            if (read == 'l') {
                mode = 0;
                line = 0;
                //printf("no suittable line");
                car.goStraight(50);
                xbee.write("line end", 8);
            } // terminate flag
            if (read == 'L') {
                uart.read(recv, sizeof(recv));
                read = *recv;
            } // ignore char L
            if (mode) {
                int i = 0;
                store1 = 1;
                while (recv[0] != '\n') {   
                    uart.read(recv, sizeof(recv));
                    if ((i < 3) && (recv[0] != '.') && (store1)) {
                        output1[i] = *recv;
                        i++;
                    } else if (recv[0] == '.') {
                        store1 = 0;
                        int k = i - 1;
                        if (k == 0) value = output1[0];
                        if (k == 1) value = 10 * output1[0] + output1[1];
                        factor = value / 100.0;
                        if (factor < 0.1 && factor > -0.1) {
                            factor = 0;
                        } 
                        // give open MV an allowable detect error so that car wont 
                        // swing from side to side 
                        car.turn(50, factor);
                    }
                }  
            }    
        }    
    }
}

void apirltag(){
    char read;
    int mode = 1;
    while(tag){
            if(uart.readable()){
                char recv[1];
                uart.read(recv, sizeof(recv));
                read = *recv;
                if(read == 'a') {
                    mode = 0;
                    tag = 0;
                    xbee.write("apirltag end", 12);
                }
                if (read == 'A') {
                    uart.read(recv, sizeof(recv));
                    read = *recv;
                }
                if (mode) {
                    int i = 0;
                    while((i < 3)&&(recv[0] != '.') && (store2)) {
                            output2[i] = *recv;
                            i++;
                    }
                    if(value == '.'){
                        store2 = 0;
                        if(i == 1) angle = output2[0];                                   // one digit
                        if(i == 2) angle = 10*output2[0] + output2[1];                    // two digits
                        if(i == 3) angle = 100*output2[0] + 10*output2[1] + output2[2];    // three digits
                        i = 0;
                        for (int j = 0; j < 3; j++) {
                            output2[j] = 0;
                        }
                    }
                    if (angle < 345 && angle >= 75) {            
                        car.turn(50,0.2);
                    } else if (angle >15 && angle < 295) {
                        car.turn(50,-0.2);
                    } else {
                        car.goStraight(50);
                    }
                }
            }
    }

}
void Ping(){
    while(1){
        ping.output();
            ping = 0; wait_us(200);
            ping = 1; wait_us(5);
            ping = 0; wait_us(5);

            ping.input();
            while(ping.read() == 0);
            t.start();
            while(ping.read() == 1);
            dis = t.read();
            dis_xbee = dis * 177700.4f;
            sprintf(buffer, "%f", dis_xbee);
            xbee.write(buffer, sizeof(buffer));           
            t.stop();
            t.reset();
    }
}


int main () {
    uart.set_baud(9600);
    pc.set_baud(9600);
    xbee.set_baud(9600);
    // thread1.start(line_detect);
    // thread2.start(apirltag);
    thread3.start(Ping);
    while(1){
        if(uart.readable()){
            char recv[1];
            uart.read(recv, sizeof(recv));
            read1 = *recv;
            if (read1 == 'L') {
                line = 1;
                line_detect();
                tag = 0;
                circle = !circle;
            }
            if (read1 == 'A'){
                tag = 1;
                apirltag();
                line = 0;
            }
            if (circle) {
                car.turn(70,0.1);
                ThisThread::sleep_for(1s);
                car.turn(80, -0.3);
                ThisThread::sleep_for(5s);
                circle = 0;
                xbee.write("circle end", 10);
            }         
        }
      }
     
}
