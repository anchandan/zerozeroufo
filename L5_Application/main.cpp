#include "wireless.h"
#include "tasks.hpp"
#include "examples/examples.hpp"
#include "LPC17xx.h"
#include "utilities.h"
#include "uart0_min.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "io.hpp"
//#include "spi_sem.h"
#include "event_groups.h"
#include "mpu_wrappers.h"
#include "task.h"
#include "command_handler.hpp"
#include "string.h"
#include "storage.hpp"
#include "i2c2.hpp"
//#include "mesh.h"

#include "GFX.hpp"
#include "RGB.hpp"
#include <stdlib.h>
#include <time.h>

RGB rgbobj;
uint32_t accel_value;

/*******COLORS*********/

#define WHITE  0xFFFF
#define GRAY   0x000F
#define MAGENTA 0xF81F
#define RED    0xF800
#define GREEN  0x202
#define CYAN   0x01FF
#define VIOLET 0xFFE0
#define BLUE   0x85DD
#define PINK   0xFF00
#define BLACK  0x0000
//#define YELLOW
/**********************/

int x_Boom, y_Boom, x_UFO_top, y_UFO_top,x_UFO_bot,y_UFO_bot;
int score=0;
QueueHandle_t orientationQ;

void TIMER0_IRQHandler (void)
{
    if((LPC_TIM0->IR & 0x01) == 0x01) // if MR0 interrupt
    {
        LPC_TIM0->IR |= 1 << 0; // Clear MR0 interrupt flag
        rgbobj.updateDisplay();
    }
}

void Wireless_Receive(void *p)
{
    mesh_packet_t pkt;
    // uint32_t message ;
    while(1)
    {
      if(wireless_get_rx_pkt(&pkt,50))
      {
         //printf("Packet received\n");
         wireless_deform_pkt(&pkt,1,&accel_value,sizeof(accel_value));
         xQueueSend(orientationQ, &accel_value, 0);
         printf("xQueueSend here: %d \t\n", accel_value);
         //printf("accel_value:%lu\n",accel_value);
         //wireless_flush_rx();
      }
    }
}

void Draw_UFO_Start(uint8_t x, uint8_t y)
{

    rgbobj.drawPixel(x,y,VIOLET);
    rgbobj.drawLine(x-1,y+1,x+1,y+1,VIOLET);
    rgbobj.drawLine(x-3,y+2,x+3,y+2,VIOLET);
    delay_ms(50);
    rgbobj.drawPixel(x,y,BLACK);
    rgbobj.drawLine(x-1,y+1,x+1,y+1,BLACK);
    rgbobj.drawLine(x-3,y+2,x+3,y+2,BLACK);
}

void Draw_UFO(uint8_t x, uint8_t y)
{
    rgbobj.drawPixel(x,y,VIOLET);
    rgbobj.drawLine(x-1,y+1,x+1,y+1,VIOLET);
    rgbobj.drawLine(x-3,y+2,x+3,y+2,VIOLET);
    x_UFO_top = x+1;
    y_UFO_top = y;
    x_UFO_bot = x+3;
    y_UFO_bot = y+2;

}

void Clear_UFO(uint8_t x, uint8_t y)
{
    rgbobj.drawPixel(x,y,BLACK);
    rgbobj.drawLine(x-1,y+1,x+1,y+1,BLACK);
    rgbobj.drawLine(x-3,y+2,x+3,y+2,BLACK);
}

void RGB_StartScreen(void *pv)
{
    srand(time(NULL));
    //Zero
    rgbobj.drawChar(3,5,'Z',GREEN,1,1 );
    rgbobj.drawChar(10,5,'E',GREEN,1,1 );
    rgbobj.drawChar(17,5,'R',GREEN,1,1 );
    rgbobj.drawChar(24,5,'0',GREEN,1,1 );
    //Zero
    rgbobj.drawChar(35,5,'Z',GREEN,1,1 );
    rgbobj.drawChar(42,5,'E',GREEN,1,1 );
    rgbobj.drawChar(49,5,'R',GREEN,1,1 );
    rgbobj.drawChar(56,5,'0',GREEN,1,1 );
    //UFO
    rgbobj.drawChar(22,15,'U',WHITE,1,1 );
    rgbobj.drawChar(29,15,'F',WHITE,1,1 );
    rgbobj.drawChar(36,15,'O',WHITE,1,1 );

    //Start
    rgbobj.drawChar(17,52,'S',RED,1,1 );
    rgbobj.drawChar(24,52,'T',RED,1,1 );
    rgbobj.drawChar(31,52,'A',RED,1,1 );
    rgbobj.drawChar(38,52,'R',RED,1,1 );
    rgbobj.drawChar(45,52,'T',RED,1,1 );

     //square
    rgbobj.drawFastHLine(0 ,0 ,64,BLUE);
    rgbobj.drawFastHLine(0 ,1 ,64,BLUE);

    rgbobj.drawFastHLine(0 ,61,64,BLUE);
    rgbobj.drawFastHLine(0 ,62,64,BLUE);

    rgbobj.drawFastVLine(0 ,0 ,64,BLUE);
    rgbobj.drawFastVLine(1 ,0 ,64,BLUE);

    rgbobj.drawFastVLine(62,0 ,64,BLUE);
    rgbobj.drawFastVLine(63,0 ,64,BLUE);

    //UFO
    int x= 7;
    int y = 50;
    int sd=0;
    while(1)
    {
        while(y > 25 & x<=58)
        {

               Draw_UFO_Start(x++,y--);

               //Clear_UFO(x,y);
               delay_ms(15);

        }
        while(sd<=25 & x<=58)
        {
            Draw_UFO_Start(x++,y++);
            sd++;
            delay_ms(15);
            if(sd==0)
            {
                x=7;
                y=50;
            }
        }
    }
}

void RGB_BoomScreen()
{
    while(1)
    {
        rgbobj.fillScreen(BLACK);

        rgbobj.drawChar(10,33,'B',RED,1,2 );
        rgbobj.drawChar(21,33,'O',RED,1,2 );
        rgbobj.drawChar(32,33,'O',RED,1,2 );
        rgbobj.drawChar(43,33,'M',RED,1,2 );
        rgbobj.drawChar(54,33,'!',RED,1,2 );
        printf("---->score:------>%d\n", score);
    }

}


void RGB_UFO(void *pv)
{
    srand(time(NULL));
    int x=3;
    int y=25;
    while(1)
    {
        rgbobj.fillScreen(BLACK);
        Draw_UFO_Start(x++,y);
        vTaskDelay(25);
        if(x>60)
        x=3;
    }
}

void Game_Screen(void *pv)
{
   srand(time(NULL));

   int direction = 0;
   int x=5,y=30,co=0;
   uint32_t orientationQ_Buffer=0;
   int x1=64, y1, height1=5;
   int x2=84, y2, height2=6;
   int x3=104, y3, height3=3;

   //Draw_UFO(5,25);
   y1= rand()%15+3;
   y2= rand()%32+3;
   y3= rand()%5+5;

   //Border
   rgbobj.drawFastHLine(0,0,64,WHITE);
   rgbobj.drawFastHLine(0,1,64,WHITE);
   rgbobj.drawFastHLine(0,62,64,WHITE);
   rgbobj.drawFastHLine(0,63,64,WHITE);
/*

   while(1)
   {
       srand(time(NULL));
       x = 5;
       //HG//y = rand()%60 + 5;
       y= 30;
       Draw_UFO(x,y);
*/
   int ob= rand()%3+1;

     while(x1>0 && x2>0 && x3>0)
       //x2=60;
   {
       score++;
       Draw_UFO(x,y);
       printf("%d\n",accel_value);
        //moving the obstacles


        rgbobj.drawObstacle(x1,y1,3,height1,RED);
        rgbobj.drawObstacle(x2,y2,3,height2,GREEN);
        rgbobj.drawObstacle(x3,y3,2,height3,BLUE);
        rgbobj.drawObstacle(x3,y3+35,2,height3,BLUE);

        if(score<=150)
            delay_ms(80);
        else if((score>150)&&(score<=225))
            delay_ms(60);
        else if(score>225)
            delay_ms(30);

        rgbobj.drawObstacle(x1,y1,3,height1,BLACK);
        rgbobj.drawObstacle(x2,y2,3,height2,BLACK);
        rgbobj.drawObstacle(x3,y3,2,height3,BLACK);
        rgbobj.drawObstacle(x3,y3+35,2,height3,BLACK);


/* check why not working
        if(        ((x_UFO==x1)&&((y_UFO>y1)&&(y_UFO<y1+height1)))
               || ((x_UFO==x2)&&((y_UFO>y2)&&(y_UFO<y2+height2)))
               || ((x_UFO==x3)&&((y_UFO>y3)&&(y_UFO<y3+height3))))
        {
           RGB_BoomScreen();
        }
*/
        if((x_UFO_top>=x1)&&((y_UFO_top>=y1)&&(y_UFO_top<=y1+height1)))
                RGB_BoomScreen();
        else if ((x_UFO_top>=x2)&&((y_UFO_top>=y2)&&(y_UFO_top<=y2+height2)))
            RGB_BoomScreen();
        else if((x_UFO_top>=x3)&&((y_UFO_top>=y3)&&(y_UFO_top<=y3+height3)))
           RGB_BoomScreen();
        if((x_UFO_bot>=x1)&&((y_UFO_bot>=y1)&&(y_UFO_bot<=y1+height1)))
                RGB_BoomScreen();
        else if ((x_UFO_bot>=x2)&&((y_UFO_bot>=y2)&&(y_UFO_bot<=y2+height2)))
            RGB_BoomScreen();
        else if((x_UFO_bot>=x3)&&((y_UFO_bot>=y3)&&(y_UFO_bot<=y3+height3)))
           RGB_BoomScreen();

        Clear_UFO(x,y);
/* randomizing the position code, moving up and down

        if(direction)
            y-- ;
        else
            y++;
*/
        xQueueReceive(orientationQ, &orientationQ_Buffer, 5);
        accel_value = orientationQ_Buffer;
        //printf("accel_value: %lu, \t co: %d\n ",accel_value,co++);

        if(accel_value == 2)
            y-- ;
        else if(accel_value == 1)
            y=y;
        else if(accel_value == 0)
            y++;


        if(y>58)
            y=58;
        else if(y<3)
            y=3;

/* randomizing the position code, moving up and down

        if(y==3)
            direction=0;
        else if(y==58)
            direction = 1;
*/
        Draw_UFO(x,y);

        x1--;
        x2--;
        x3--;

        if(x1==0)
        {
           x1=64;
           y1= rand()%32+3;
           height1= rand()%15+5;
        }
        if(x2==0)
        {
           x2=64;
           y2= rand()%32+3;
           height2= rand()%15+5;
        }
        if(x3==0)
        {
           x3=64;
           y3= rand()%5+5;
           height3= rand()%5+7;
        }
   }

}
int main(void)
{

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));

    orientationQ = xQueueCreate(1, sizeof(accel_value));

#if 1
        rgbobj.init(32, 19, 20, 22, 23, 28, 6, 29, 7, true, 64);
        isr_register(TIMER0_IRQn, TIMER0_IRQHandler);  //Registers Timer Interrupt
        rgbobj.begin();
        //xTaskCreate(RGB_StartScreen,"Start_Screen",256,NULL,1,NULL);
        //xTaskCreate(RGB_UFO, "UFO", 256, NULL, 1, NULL);
        xTaskCreate(Game_Screen, "Obstacle", 256, NULL, PRIORITY_LOW, NULL);
        xTaskCreate(Wireless_Receive,"Wireless_Receive",256, NULL, PRIORITY_HIGH, NULL);

#endif

    #if 0
    const bool run_1Khz = false;
    scheduler_add_task(new periodicSchedulerTask(run_1Khz));
    #endif

    #if 0
        scheduler_add_task(new example_io_demo());
    #endif

    #if 0
        scheduler_add_task(new example_task());
        scheduler_add_task(new example_alarm());
        scheduler_add_task(new example_logger_qset());
        scheduler_add_task(new example_nv_vars());
    #endif

    #if 0
        scheduler_add_task(new queue_tx());
        scheduler_add_task(new queue_rx());
    #endif

    #if 0
        scheduler_add_task(new producer());
        scheduler_add_task(new consumer());
    #endif

    #if 0
        Uart3 &u3 = Uart3::getInstance();
        u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
        scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
    #endif
    #if 0
        // send task
        uint8_t addr=106;
        const char Hops=0;
      //  mesh_packet_t mesh;
       // uint8_t send = 22;
        char str = 2;
        printf("\n outside while");
        while(1)
        {
          mesh_send(addr, mesh_pkt_nack, &str, 1, Hops);
          printf("\nSending");
          delay_ms(1000);
          //vTaskDelay(1000);
        }
    #endif


    scheduler_start(); ///< This shouldn't return
    return -1;
}
