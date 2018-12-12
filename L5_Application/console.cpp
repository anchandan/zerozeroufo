#include "console.hpp"

RGB rgb;
SemaphoreHandle_t update_display_semphr;
QueueHandle_t orientation_q;
QueueHandle_t control_q;

int x_Boom, y_Boom, x_UFO_top, y_UFO_top,x_UFO_bot,y_UFO_bot;
int score=0;
bool boom_flag = false;
TaskHandle_t gameplay_h;
TaskHandle_t start_h;
TaskHandle_t BoomScreenHandle;

void timer0_ISR(void)
{
#if 0
    portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;

    if((LPC_TIM0->IR & 0x01) == 0x01) { /* if MR0 interrupt */
        /* Clear MR0 interrupt flag */
        xSemaphoreGiveFromISR(update_display_semphr, &pxHigherPriorityTaskWoken);
        LPC_TIM0->IR |= (1 << 0);
        //TODO task yield (check pxhigherprioritytaskwoken) check canvas RTOS slides
    }
#else
    if((LPC_TIM0->IR & 0x01) == 0x01) { /* if MR0 interrupt */
        /* Clear MR0 interrupt flag */
        rgb.updateDisplay();
        LPC_TIM0->IR |= 1 << 0;
    }
#endif
}

/* TODO figure out why display is so slow/choppy when we use semaphore */
void update_display_task(void *p)
{
    while (1) {
        if (xSemaphoreTake(update_display_semphr, portMAX_DELAY)) {
            rgb.updateDisplay();
        }
    }
}

void receive_message(void *p)
{
    mesh_packet_t pkt;
#if 0
    uint32_t opcode;
    uint32_t data;
#endif
#if 1
    uint32_t accel_value;
    int game_session = 0;
#endif

    while (1) {
#if 0
        if (wireless_get_rx_pkt(&pkt, 100)) {
            wireless_deform_pkt(&pkt, 2,
                    &opcode, sizeof(opcode),
                    &data, sizeof(data));
            if (opcode == ZZU_DATA) {
                xQueueSend(orientation_q, &data, 0);
            } else if (opcode == ZZU_CTRL) {
                xQueueSend(control_q, &data, 0);
            } else {
                u0_dbg_printf("ERROR: invalid opcode %d\n", opcode);
            }
        }
#else
        if (wireless_get_rx_pkt(&pkt, 80)) {
            //u0_dbg_printf("Packet received\n");
            wireless_deform_pkt(&pkt,1,&accel_value,sizeof(accel_value));
        //         xQueueSend(orientation_q, &accel_value, 0);
            //u0_dbg_printf("%d\n", accel_value);
            //u0_dbg_printf("accel_value:%lu\n",accel_value);
            //wireless_flush_rx();

            if (game_session>=1) {
                xQueueSend(orientation_q, &accel_value, 0);
            }

            if (accel_value == 100) {
                if (game_session == 0) {
                    u0_dbg_printf("in game session == 0 \n");
                    game_session = 2;
                    vTaskSuspend(start_h);
                    xTaskCreate(gameplay, "gameplay", 1024, NULL, PRIORITY_MEDIUM, &gameplay_h);
                } else if(game_session == 1) {
                    u0_dbg_printf("in game session == 1 \n");
                    game_session = 2;

                    rgb.fillScreen(BLACK);
                    boom_flag = true;
                    xTaskCreate(gameplay,"gameplay",1024,NULL,PRIORITY_MEDIUM,&gameplay_h);
                    //vTaskResume(gameplay_h);
                    vTaskSuspend(BoomScreenHandle);
                    vTaskSuspend(start_h);
                } else if (game_session == 2) {
                    u0_dbg_printf("in game session == 2 \n");
                    game_session = 1;
                    vTaskDelete(gameplay_h);
                    rgb.fillScreen(BLACK);
                    vTaskResume(start_h);
                    vTaskSuspend(BoomScreenHandle);
                }
            }
            u0_dbg_printf("ok\n");
        }
#endif
    }
}

void Draw_UFO_Start(uint8_t x, uint8_t y)
{

    rgb.drawPixel(x,y,VIOLET);
    rgb.drawLine(x-1,y+1,x+1,y+1,VIOLET);
    rgb.drawLine(x-3,y+2,x+3,y+2,VIOLET);
    delay_ms(50);
    rgb.drawPixel(x,y,BLACK);
    rgb.drawLine(x-1,y+1,x+1,y+1,BLACK);
    rgb.drawLine(x-3,y+2,x+3,y+2,BLACK);
}

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

void start_screen(void *p)
{
    int x, y, sd;

    srand(time(NULL));

    /* title */
    rgb.drawChar(3, 5, 'Z', GREEN, 1, 1);
    rgb.drawChar(10, 5, 'E', GREEN, 1, 1);
    rgb.drawChar(17, 5, 'R', GREEN, 1, 1);
    rgb.drawChar(24, 5, '0', GREEN, 1, 1);
    rgb.drawChar(35, 5, 'Z', GREEN, 1, 1);
    rgb.drawChar(42, 5, 'E', GREEN, 1, 1);
    rgb.drawChar(49, 5, 'R', GREEN, 1, 1);
    rgb.drawChar(56, 5, '0', GREEN, 1, 1);
    rgb.drawChar(22, 15, 'U', WHITE, 1, 1);
    rgb.drawChar(29, 15, 'F', WHITE, 1, 1);
    rgb.drawChar(36, 15, 'O', WHITE, 1, 1);

    /* start */
    rgb.drawChar(17,52,'S',RED,1,1);
    rgb.drawChar(24,52,'T',RED,1,1);
    rgb.drawChar(31,52,'A',RED,1,1);
    rgb.drawChar(38,52,'R',RED,1,1);
    rgb.drawChar(45,52,'T',RED,1,1);

    /* border */
    rgb.drawFastHLine(0, 0, 64, BLUE);
    rgb.drawFastHLine(0, 1, 64, BLUE);
    rgb.drawFastHLine(0, 62, 64, BLUE);
    rgb.drawFastHLine(0, 63, 64, BLUE);
    rgb.drawFastVLine(0, 0, 64, BLUE);
    rgb.drawFastVLine(1, 0, 64, BLUE);
    rgb.drawFastVLine(62, 0, 64, BLUE);
    rgb.drawFastVLine(63, 0, 64, BLUE);

    /* UFO animation */
#if 1
    while (1) {
        x = 7;
        y = 50;
        sd = 0;
        while (y > 25 && x <=58) {
            Draw_UFO_Start(x++, y--);

            delay_ms(15);
        }
        while (sd <= 25 && x <= 58) {
            Draw_UFO_Start(x++, y++);
            sd++;
            delay_ms(15);
            if (sd==0) {
                x = 7;
                y = 50;
            }
        }
    }
#else
    //rgb.drawPixel(10,10,GREEN);
    //rgb.drawPixel(10,10,GREEN);

    while (y > 25 & x<=58) {
        Draw_UFO_Start(x++,y--);

        //Clear_UFO(x,y);
        delay_ms(15);
    }
    while(sd<=25 & x<=58) {
        Draw_UFO_Start(x++,y++);
        sd++;
        delay_ms(15);
        if(sd==0) {
            x=7;
            y=50;
        }
    }
#endif
}

void RGB_BoomScreen(void *p)
{
    char rem_Char[1];
    int x=60;
    int rem_Int;
    rgb.fillScreen(BLACK);

    while (1) {
#if 0
        rgb.fillScreen(BLACK);

        rgb.drawChar(10,33,'B',RED,1,2 );
        rgb.drawChar(21,33,'O',RED,1,2 );
        rgb.drawChar(32,33,'O',RED,1,2 );
        rgb.drawChar(43,33,'M',RED,1,2 );
        rgb.drawChar(54,33,'!',RED,1,2 );
        u0_dbg_printf("---->score:------>%d\n", score);
#else
        boom_flag = true;
        rgb.drawChar(10,25,'B',RED,1,2);
        rgb.drawChar(21,25,'O',RED,1,2);
        rgb.drawChar(32,25,'O',RED,1,2);
        rgb.drawChar(43,25,'M',RED,1,2);
        rgb.drawChar(54,25,'!',RED,1,2);
        u0_dbg_printf("---->score:------>%d\n", score);

        rgb.drawChar(6,45,'S',GREEN,1,1);
        rgb.drawChar(12,45,'C',GREEN,1,1);
        rgb.drawChar(18,45,'O',GREEN,1,1);
        rgb.drawChar(24,45,'R',GREEN,1,1);
        rgb.drawChar(30,45,'E',GREEN,1,1);
        rgb.drawChar(34,45,':',GREEN,1,1);

        //HG: Code for displaying score

        x=x-6;
        rem_Int=score%10;
        itoa(rem_Int,rem_Char,10);

        if(score>0)
        {
            rgb.drawChar(x,45,rem_Char[0],GREEN,1,1 );
        }
        else
            vTaskDelay(3000);
        score=score/10;
#endif
    }

}
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

void gameplay(void *p)
{
#if 0
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
        //u0_dbg_printf("accel_value: %lu, \t co: %d\n ",accel_value,co++);

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
#else
    uint32_t accel_value;
    rgb.fillScreen(BLACK);
    srand(time(NULL));

    int direction = 0;
    int x=5,y=30,co=0;
    uint32_t orientationQ_Buffer=0;
    int x1=64, y1, height1=5;
    int x2=74, y2, height2=6;
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

    u0_dbg_printf("gamescween\n");
    while (x1>0 && x2>0 && x3>0) {
         //x2=60;
          // rgbobj.drawPixel(10,31,GREEN);//HG
        score++;
        Draw_UFO(x,y);
           //u0_dbg_printf("%d\n",accel_value);
        //moving the obstacles
        rgb.drawObstacle(x1,y1,3,height1,RED);
        //rgb.drawObstacle(x1,y1+20,3,height1+5,RED);
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
        //rgbobj.drawObstacle(x1,y1+20,3,height1+5,BLACK);
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
        u0_dbg_printf("Boom_flag : %d \n",boom_flag);
        if (boom_flag == false) {
            if((x_UFO_top>=x1)&&((y_UFO_top>=y1)&&(y_UFO_top<=y1+height1))) {
                x_UFO_top = 0; y_UFO_top = 0;
                u0_dbg_printf("In boomscreen\r\nx_UFO_top:%d  y_UFO_top:%d\n x1:%d y1:%d \n",x_UFO_top,y_UFO_top,x1,y1);
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
            } else if ((x_UFO_top>=x2)&&((y_UFO_top>=y2)&&(y_UFO_top<=y2+height2))) {
                x_UFO_top = 0; y_UFO_top = 0;
                u0_dbg_printf("In boomscreen\r\n x_UFO_top:%d  y_UFO_top:%d\n x2:%d y2:%d \n",x_UFO_top,y_UFO_top,x2,y2);
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
            } else if((x_UFO_top>=x3)&&((y_UFO_top>=y3)&&(y_UFO_top<=y3+height3))) {
                x_UFO_top = 0; y_UFO_top = 0;
                u0_dbg_printf("In boomscreen\r\n x_UFO_top:%d  y_UFO_top:%d\n x3:%d y3:%d \n",x_UFO_top,y_UFO_top,x3,y3);
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
            }

            if((x_UFO_bot>=x1)&&((y_UFO_bot>=y1)&&(y_UFO_bot<=y1+height1))) {
                x_UFO_bot = 0; y_UFO_bot = 0;
                u0_dbg_printf("In boomscreen\r\n x_UFO_bot:%d  y_UFO_bot:%d\n x1:%d y1:%d \n",x_UFO_bot,y_UFO_bot ,x1,y1);
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
            } else if ((x_UFO_bot>=x2)&&((y_UFO_bot>=y2)&&(y_UFO_bot<=y2+height2))) {
                x_UFO_bot = 0; y_UFO_bot = 0;
                u0_dbg_printf("In boomscreen\r\n x_UFO_bot:%d  y_UFO_bot:%d\n x2:%d y2:%d \n",x_UFO_bot,y_UFO_bot , x2,y3);
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
            } else if((x_UFO_bot>=x3)&&((y_UFO_bot>=y3)&&(y_UFO_bot<=y3+height3))) {
                x_UFO_bot = 0; y_UFO_bot = 0;
                u0_dbg_printf("In boomscreen\r\n x_UFO_bot:%d  y_UFO_bot:%d\n x3:%d y3:%d \n",x_UFO_bot,y_UFO_bot,x3,y3);
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
            }
        }
        boom_flag = false;
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

        if(accel_value==0)
            y++ ;
        else if(accel_value==2)
            y--;

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

        if(x1==0) {
            x1=64;
            y1= rand()%32+3;
            height1= rand()%15+5;
        }
        if(x2==0) {
            x2=64;
            y2= rand()%32+3;
            height2= rand()%15+5;
        }
        if(x3==0) {
            x3=64;
            y3= rand()%5+5;
            height3= rand()%5+7;
        }
    }
#endif
}


void Pixel_Test (void*pv)
{
    //int x=0;
    int y=0;
    while (1) {

/*        rgbobj.drawFastHLine(0,30,64,RED);
        delay_ms(3000);
        rgbobj.drawFastHLine(0,31,64,GREEN);
        delay_ms(500);
        rgbobj.drawFastHLine(0,32,64,BLUE);
        delay_ms(500);
        rgbobj.drawFastHLine(0,33,66,WHITE);
        delay_ms(500);*/

        rgb.drawFastHLine(1,y,63,RED);
        delay_ms(500);
        y++;
    }
}


#if 0
void console(void *p) /* HIGH priority */
{
    mesh_packet_t pkt;
    char header[64];
    int x;
    /* TODO handle screen output and generate cave map */

    while (1) {
        if (wireless_get_rx_pkt(&pkt, 100)) {
            wireless_deform_pkt(&pkt, 2,
                                &header, sizeof(header),
                                &x, sizeof(x));
        }
        ///vTaskDelay(20);
    }
}
#endif
