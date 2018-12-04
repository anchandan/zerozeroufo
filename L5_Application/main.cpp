/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
 /**
 * @file
 * @brief This is the application entry point.
 *          FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 *          @see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include "sys_config.h"
#include "tasks.hpp"
#include "examples/examples.hpp"

#ifdef ZZU_CONTROLLER
#include "controller.hpp"
#endif /* ZZU_CONTROLLER */

#ifdef ZZU_CONSOLE
#include "console.hpp"
#endif /* ZZU_CONSOLE */

#if 0
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
#endif

int x_Boom, y_Boom, x_UFO_top, y_UFO_top,x_UFO_bot,y_UFO_bot;
int score=0;

#ifdef ZZU_CONSOLE
void Draw_UFO(uint8_t x, uint8_t y)
{
    rgb.drawPixel(x,y,VIOLET);
    rgb.drawLine(x-1,y+1,x+1,y+1,VIOLET);
    rgb.drawLine(x-3,y+2,x+3,y+2,VIOLET);
    x_UFO_top = x+1;
    y_UFO_top = y;
    x_UFO_bot = x+3;
    y_UFO_bot = y+2;

}

void Clear_UFO(uint8_t x, uint8_t y)
{
    rgb.drawPixel(x,y,BLACK);
    rgb.drawLine(x-1,y+1,x+1,y+1,BLACK);
    rgb.drawLine(x-3,y+2,x+3,y+2,BLACK);
}


void RGB_BoomScreen()
{
    while(1)
    {
        rgb.fillScreen(BLACK);

        rgb.drawChar(10,33,'B',RED,1,2 );
        rgb.drawChar(21,33,'O',RED,1,2 );
        rgb.drawChar(32,33,'O',RED,1,2 );
        rgb.drawChar(43,33,'M',RED,1,2 );
        rgb.drawChar(54,33,'!',RED,1,2 );
        u0_dbg_printf("---->score:------>%d\n", score);
    }

}


#if 0
void RGB_UFO(void *pv)
{
    srand(time(NULL));
    int x=3;
    int y=25;
    while(1)
    {
        rgb.fillScreen(BLACK);
        Draw_UFO_Start(x++,y);
        vTaskDelay(25);
        if(x>60)
        x=3;
    }
}
#endif

void Game_Screen(void *pv)
{
    uint32_t accel_value;
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
   rgb.drawFastHLine(0,0,64,WHITE);
   rgb.drawFastHLine(0,1,64,WHITE);
   rgb.drawFastHLine(0,62,64,WHITE);
   rgb.drawFastHLine(0,63,64,WHITE);
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
       u0_dbg_printf("%d\n",accel_value);
        //moving the obstacles


        rgb.drawObstacle(x1,y1,3,height1,RED);
        rgb.drawObstacle(x2,y2,3,height2,GREEN);
        rgb.drawObstacle(x3,y3,2,height3,BLUE);
        rgb.drawObstacle(x3,y3+35,2,height3,BLUE);

        if(score<=150)
            delay_ms(80);
        else if((score>150)&&(score<=225))
            delay_ms(60);
        else if(score>225)
            delay_ms(30);

        rgb.drawObstacle(x1,y1,3,height1,BLACK);
        rgb.drawObstacle(x2,y2,3,height2,BLACK);
        rgb.drawObstacle(x3,y3,2,height3,BLACK);
        rgb.drawObstacle(x3,y3+35,2,height3,BLACK);


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
        xQueueReceive(orientation_q, &orientationQ_Buffer, 5);
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
#endif /* ZZU_CONSOLE */

int main(void)
{
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
    scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));

#if 0
    const bool run_1Khz = false;
    scheduler_add_task(new periodicSchedulerTask(run_1Khz));

    Uart3 &u3 = Uart3::getInstance();
    u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
    scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
#endif

#ifdef ZZU_CONTROLLER
    // send task
    uint8_t addr=106;
    const char Hops=0;
    //  mesh_packet_t mesh;
    // uint8_t send = 22;
    char str = 2;
    printf("\n outside while");
    while(1) {
        mesh_send(addr, mesh_pkt_nack, &str, 1, Hops);
        printf("\nSending");
        delay_ms(1000);
        //vTaskDelay(1000);
    }
    xTaskCreate(controller, (const char *)"controller", 2048, NULL, 2, NULL);
#endif /* ZZU_CONTROLLER */

#ifdef ZZU_CONSOLE
    update_display_semphr = xSemaphoreCreateBinary();
    orientation_q = xQueueCreate(1, sizeof(uint32_t));
    control_q = xQueueCreate(1, sizeof(uint32_t));

    isr_register(TIMER0_IRQn, timer0_ISR);
    rgb.init(32, 19, 20, 22, 23, 28, 6, 29, 7, true, 64);
    rgb.begin();

    //xTaskCreate(update_display_task, "update display", 256, NULL, PRIORITY_HIGH, NULL);
    xTaskCreate(title_screen, "title screen", 256, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(RGB_UFO, "UFO", 256, NULL, 1, NULL);
    //xTaskCreate(Game_Screen, "Obstacle", 256, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(receive_msg, "receive messages", 256, NULL, PRIORITY_CRITICAL, NULL);

    //xTaskCreate(console, (const char *)"console", 2048, NULL, 2, NULL);
#endif /* ZZU_CONSOLE */

    scheduler_start(); ///< This shouldn't return
    return -1;
}
