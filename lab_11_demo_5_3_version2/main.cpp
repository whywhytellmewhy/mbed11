#include "mbed.h"
#include "mbed_rpc.h"

//--------------(added)---------------
#include "stm32l475e_iot01_accelero.h"
#include <string.h>

using namespace std::chrono;
//------------------------------------


static BufferedSerial pc(STDIO_UART_TX, STDIO_UART_RX);
static BufferedSerial xbee(D1, D0);

EventQueue queue(128 * EVENTS_EVENT_SIZE);
EventQueue queues(128 * EVENTS_EVENT_SIZE);
Thread t;
//----------(added)-----------------------------
Thread thread_a;
Timeout flipper;
//----------------------------------------------

RpcDigitalOut myled1(LED1,"myled1");
RpcDigitalOut myled2(LED2,"myled2");
RpcDigitalOut myled3(LED3,"myled3");


void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);

//-----------------(added)------------------
void accelerometer_meature(Arguments *in, Reply *out);
//void flip();

RPCFunction rpcaccelerometer(&accelerometer_meature, "ACC");
int16_t eeeDataXYZ[3] = {0};
int idR[32] = {0};
int indexR = 0;
//char xbee_reply[4];
char Data_string[15]={'\0'};
int i=0;
int get_a=0;


void meature(){
   ThisThread::sleep_for(500ms);
   BSP_ACCELERO_AccGetXYZ(eeeDataXYZ);
    printf("accelerometer_meature!!!\n");
    printf("x: %d,y: %d,z: %d\n",eeeDataXYZ[0],eeeDataXYZ[1],eeeDataXYZ[2]);
    sprintf(Data_string, "%5d%5d%5d", eeeDataXYZ[0],eeeDataXYZ[1],eeeDataXYZ[2]);



   //printf("1:%c 2:%c 3:%c 4:%c 5:%c 6:%c\n",Data_string[0],Data_string[1],Data_string[2],Data_string[3],Data_string[4],Data_string[5]);
   //for(i=0;i<=15;i=i+1){
   //   printf("%d:%c ",i,Data_string[i]);
   //}
   //printf("\n");
   //xbee.write(Data_string, 15);
   //xbee.write("12312312312312\0", 14);
   //printf("Send Data_string!!!");
}

void flip()
{
   queues.call(meature);
}

void accelerometer_meature(Arguments *in, Reply *out){
   printf("accelerometer!!!\n");
   //int16_t eeeDataXYZ[3] = {0};
   //BSP_ACCELERO_Init();
get_a=1;
printf("get_a=1\n");


   
   //ThisThread::sleep_for(3s);
   //ThisThread::sleep_for(500ms);

   thread_a.start(callback(&queues, &EventQueue::dispatch_forever));
   flipper.attach(&flip, 2s);

//    BSP_ACCELERO_AccGetXYZ(eeeDataXYZ);
//    printf("accelerometer_meature!!!\n");
//    printf("x: %d,y: %d,z: %d\n",eeeDataXYZ[0],eeeDataXYZ[1],eeeDataXYZ[2]);
//    sprintf(Data_string, "%5d%5d%5d", eeeDataXYZ[0],eeeDataXYZ[1],eeeDataXYZ[2]);
   //printf("1:%c 2:%c 3:%c 4:%c 5:%c 6:%c\n",Data_string[0],Data_string[1],Data_string[2],Data_string[3],Data_string[4],Data_string[5]);
//   for(i=0;i<=15;i=i+1){
//      printf("%d:%c ",i,Data_string[i]);
//   }
//   printf("\n");
    //itoa(number,string,10);
    //xbee.write((char)pDataXYZ[0], sizeof((char)pDataXYZ[0]));
    //xbee.write("12\n", 3);
   //reply_messange(xbee_reply, "(%d,%d,%d)",pDataXYZ[0],pDataXYZ[1],pDataXYZ[2]);
}




//-----------------------------------------

int main(){
   BSP_ACCELERO_Init();
   pc.set_baud(9600);

   //char xbee_reply[4];
   char xbee_reply[4];

   xbee.set_baud(9600);
   xbee.write("+++", 3);
   xbee.read(&xbee_reply[0], sizeof(xbee_reply[0]));
   xbee.read(&xbee_reply[1], sizeof(xbee_reply[1]));
   if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
      printf("enter AT mode.\r\n");
      xbee_reply[0] = '\0';
      xbee_reply[1] = '\0';
   }else{
       printf("Noooooooo.\r\n");
       printf("%c %c.\r\n",xbee_reply[0],xbee_reply[1]);
   }

   xbee.write("ATMY 0x256\r\n", 12);
   reply_messange(xbee_reply, "setting MY : 0x256");
   xbee.write("ATDL 0x156\r\n", 12);
   reply_messange(xbee_reply, "setting DL : 0x156");

   xbee.write("ATID 0x1\r\n", 10);
   reply_messange(xbee_reply, "setting PAN ID : 0x1");

   xbee.write("ATWR\r\n", 6);
   reply_messange(xbee_reply, "write config");

   xbee.write("ATMY\r\n", 6);
   check_addr(xbee_reply, "MY");

   xbee.write("ATDL\r\n", 6);
   check_addr(xbee_reply, "DL");

   xbee.write("ATCN\r\n", 6);
   reply_messange(xbee_reply, "exit AT mode");

   while(xbee.readable()){
      char *k = new char[1];
      xbee.read(k,1);
      printf("clear\r\n");
   }

   // start
   printf("start\r\n");
   t.start(callback(&queue, &EventQueue::dispatch_forever));

   //----------------------(added)----------------------------------
   printf("Start accelerometer init\n");
    //BSP_ACCELERO_Init();
    //---------------------------------------------------------------

   // Setup a serial interrupt function of receiving data from xbee
   xbee.set_blocking(false);
   xbee.sigio(mbed_event_queue()->event(xbee_rx_interrupt));

   //-----------------(added)---------------------
   /*while(1){
       printf("111111111111!\n");
      if(get_a==1){
         printf("accelerometer_meature!!!\n");
          BSP_ACCELERO_AccGetXYZ(eeeDataXYZ);
         
         printf("x: %d,y: %d,z: %d\n",eeeDataXYZ[0],eeeDataXYZ[1],eeeDataXYZ[2]);
         
      }
      ThisThread::sleep_for(1s);
   }*/
   //----------------------------------------
}

//void xbee_rx_interrupt(void)
//{
//   queue.call(&xbee_rx);
//}

//void xbee_rx(void)
void xbee_rx_interrupt(void)
{
   //printf("111111111111111111\n");
   char buf[100] = {0};
   char outbuf[100] = {0};
   while(xbee.readable()){
      for (int i=0; ; i++) {
         char *recv = new char[1];
         xbee.read(recv, 1);
         buf[i] = *recv;
         if (*recv == '\r') {
         break;
         }
      }

      RPC::call(buf, outbuf);

      printf("%s\r\n", outbuf);
      ThisThread::sleep_for(1s);
   }
   /*static int i = 0;
   static char buf[100] = {0};
   while(xbee.readable()){
      char *c = new char[1];
      xbee.read(c, 1);
      if(*c!='\r' && *c!='\n'){
         buf[i] = *c;
         i++;
         buf[i] = '\0';
      }else if((*c == '\r' || *c == '\n') && i == 0){ // ignore redundant char in buffer

      }
      else
      {
         i = 0;
         printf("Get: %s\r\n", buf);
         printf("%s\r\n", buf);
         xbee.write("123\n", 4);
         xbee.write(buf, sizeof(buf));
         xbee.write("123\n", 4);
      }
   }
   ThisThread::sleep_for(100ms);*/
   //ThisThread::sleep_for(100ms);
   //xbee.write("123\n", 4);

}

void reply_messange(char *xbee_reply, char *messange){
   xbee.read(&xbee_reply[0], 1);
   xbee.read(&xbee_reply[1], 1);
   xbee.read(&xbee_reply[2], 1);
   if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
      printf("%s\r\n", messange);
      xbee_reply[0] = '\0';
      xbee_reply[1] = '\0';
      xbee_reply[2] = '\0';
   }
}

void check_addr(char *xbee_reply, char *messenger){
   xbee.read(&xbee_reply[0], 1);
   xbee.read(&xbee_reply[1], 1);
   xbee.read(&xbee_reply[2], 1);
   xbee.read(&xbee_reply[3], 1);
   printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
   xbee_reply[0] = '\0';
   xbee_reply[1] = '\0';
   xbee_reply[2] = '\0';
   xbee_reply[3] = '\0';
}

/*#include "mbed.h"
#include "mbed_rpc.h"


static BufferedSerial pc(STDIO_UART_TX, STDIO_UART_RX);
static BufferedSerial xbee(D1, D0);

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

RpcDigitalOut myled1(LED1,"myled1");
RpcDigitalOut myled2(LED2,"myled2");
RpcDigitalOut myled3(LED3,"myled3");

void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);

int main(){

   pc.set_baud(9600);

   char xbee_reply[4];

   xbee.set_baud(9600);
   xbee.write("+++", 3);
   xbee.read(&xbee_reply[0], sizeof(xbee_reply[0]));
   xbee.read(&xbee_reply[1], sizeof(xbee_reply[1]));
   if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
      printf("enter AT mode.\r\n");
      xbee_reply[0] = '\0';
      xbee_reply[1] = '\0';
   }

   xbee.write("ATMY <Remote MY>\r\n", 12);
   reply_messange(xbee_reply, "setting MY : <Remote MY>");
   xbee.write("ATDL <Remote DL>\r\n", 12);
   reply_messange(xbee_reply, "setting DL : <Remote DL>");

   xbee.write("ATID <PAN ID>\r\n", 10);
   reply_messange(xbee_reply, "setting PAN ID : <PAN ID>");

   xbee.write("ATWR\r\n", 6);
   reply_messange(xbee_reply, "write config");

   xbee.write("ATMY\r\n", 6);
   check_addr(xbee_reply, "MY");

   xbee.write("ATDL\r\n", 6);
   check_addr(xbee_reply, "DL");

   xbee.write("ATCN\r\n", 6);
   reply_messange(xbee_reply, "exit AT mode");

   while(xbee.readable()){
      char *k = new char[1];
      xbee.read(k,1);
      printf("clear\r\n");
   }

   // start
   printf("start\r\n");
   t.start(callback(&queue, &EventQueue::dispatch_forever));

   // Setup a serial interrupt function of receiving data from xbee
   xbee.set_blocking(false);
   xbee.sigio(mbed_event_queue()->event(xbee_rx_interrupt));
}

void xbee_rx_interrupt(void)
{
   queue.call(&xbee_rx);
}

void xbee_rx(void)
{
   char buf[100] = {0};
   char outbuf[100] = {0};
   while(xbee.readable()){
      for (int i=0; ; i++) {
         char *recv = new char[1];
         xbee.read(recv, 1);
         buf[i] = *recv;
         if (*recv == '\r') {
         break;
         }
      }

      RPC::call(buf, outbuf);

      printf("%s\r\n", outbuf);
      ThisThread::sleep_for(1s);
   }

}

void reply_messange(char *xbee_reply, char *messange){
   xbee.read(&xbee_reply[0], 1);
   xbee.read(&xbee_reply[1], 1);
   xbee.read(&xbee_reply[2], 1);
   if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
      printf("%s\r\n", messange);
      xbee_reply[0] = '\0';
      xbee_reply[1] = '\0';
      xbee_reply[2] = '\0';
   }
}

void check_addr(char *xbee_reply, char *messenger){
   xbee.read(&xbee_reply[0], 1);
   xbee.read(&xbee_reply[1], 1);
   xbee.read(&xbee_reply[2], 1);
   xbee.read(&xbee_reply[3], 1);
   printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
   xbee_reply[0] = '\0';
   xbee_reply[1] = '\0';
   xbee_reply[2] = '\0';
   xbee_reply[3] = '\0';
}*/