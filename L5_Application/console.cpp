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
uint16_t x_c, y_c;

#if 0 /* NOTE could implement obstacle generation with ring array(s) */
Obstacle obstacle_ring[MAX_OBSTACLES];
uint32_t or_head = 0, or_tail = 0, or_count = 0;
#endif
uint32_t gameplay_timers[MAX_SPEED] = { 0 };

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
            wireless_deform_pkt(&pkt,1,&accel_value,sizeof(accel_value));
        //         xQueueSend(orientation_q, &accel_value, 0);

            if (game_session>=1) {
                xQueueSend(orientation_q, &accel_value, 0);
            }

            if (accel_value == 100) {
                if (game_session == 0) {
                    //u0_dbg_printf("in game session == 0 \n");
                    game_session = 2;
                    vTaskSuspend(start_h);
                    xTaskCreate(gameplay, "gameplay", 2048, NULL, PRIORITY_MEDIUM, &gameplay_h);
                } else if(game_session == 1) {
                    //u0_dbg_printf("in game session == 1 \n");
                    game_session = 2;

                    rgb.fillScreen(BLACK);
                    boom_flag = true;
                    xTaskCreate(gameplay, "gameplay", 2048, NULL, PRIORITY_MEDIUM, &gameplay_h);
                    //vTaskResume(gameplay_h);
                    vTaskSuspend(BoomScreenHandle);
                    vTaskSuspend(start_h);
                } else if (game_session == 2) {
                    //u0_dbg_printf("in game session == 2 \n");
                    game_session = 1;
                    vTaskDelete(gameplay_h);
                    rgb.fillScreen(BLACK);
                    vTaskResume(start_h);
                    vTaskSuspend(BoomScreenHandle);
                }
            }
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

bool ufo_draw_collision(uint8_t x, uint8_t y)
{
    bool collision = false;
    //uint16_t x_c, y_c;

    //collision = rgb.drawPixel(x, y, VIOLET) || collision;
    if (rgb.drawPixel(x, y, VIOLET)) {
        x_c = x;
        y_c = y;
        collision = true;
    }
    collision = rgb.drawLineCollision(x-1, y+1, x+1, y+1, VIOLET, &x_c, &y_c) || collision;
    collision = rgb.drawLineCollision(x-3, y+2, x+3, y+2, VIOLET, &x_c, &y_c) || collision;

    return collision;
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

    while (1) {
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
        x = 7;
        y = 50;
        sd = 0;
        while (y > 25 && x <=58) {
            Draw_UFO_Start(x++, y--);
            vTaskDelay(20);
        }
        while (sd <= 25 && x <= 58) {
            Draw_UFO_Start(x++, y++);
            sd++;
            vTaskDelay(20);
            if (sd == 0) {
                x = 7;
                y = 50;
            }
        }
        vTaskDelay(20);
    }
}

void RGB_BoomScreen(void *p)
{
    char rem_Char[1];
    int x=60;
    int rem_Int;
    rgb.fillScreen(BLACK);

    while (1) {
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
    }
}

uint32_t get_rand(uint32_t mod, uint32_t off)
{
    return (rand() % mod) + off;
}

void game_tick()
{
    uint32_t mod, time;

    for (uint32_t i = 0; i < MAX_SPEED; i++) {
        mod = 1 << (MAX_SPEED - i - 1);
        time = gameplay_timers[i];
        gameplay_timers[i] = (time + 1) % mod;
    }
}

void gameplay(void *p)
{
    uint32_t accel_value;
    bool collision_flag = false, token_flag = false;

    rgb.fillScreen(BLACK);

    /* initialize map objects */
    /* TODO handle logic more regularly */
    int x = 5, y = 30;
    uint32_t orientationQ_Buffer=0;
    int x3 = 104, y3 = get_rand(5, 5), height3 = 3;
    Obstacle ob1 = Obstacle();
    Obstacle ob2 = Obstacle();
    Obstacle ob3 = Obstacle();
    Obstacle ob4 = Obstacle();
    Coin coin1 = Coin();
    Coin coin2 = Coin();
    Coin coin3 = Coin();

    while (1) {
        /* border/tunnel */
        rgb.drawFastHLine(0,0,64,WHITE);
        rgb.drawFastHLine(0,1,64,WHITE);
        rgb.drawFastHLine(0,62,64,WHITE);
        rgb.drawFastHLine(0,63,64,WHITE);

        /* draw obstacles */
        rgb.drawObstacle(x3,y3,2,height3,BLUE, true);
        rgb.drawObstacle(x3,y3+35,2,height3,BLUE, true);
        ob1.draw();
        ob2.draw();
        ob3.draw();
        ob4.draw();
        coin1.draw();
        coin2.draw();
        coin3.draw();

        collision_flag = ufo_draw_collision(x, y);

        /* game tick */
        vTaskDelay(20);
        game_tick();

        /* clear objects for next tick */
        rgb.drawObstacle(x3,y3,2,height3,BLACK, true);
        rgb.drawObstacle(x3,y3+35,2,height3,BLACK, true);
        ob1.erase();
        ob2.erase();
        ob3.erase();
        ob4.erase();
        coin1.erase();
        coin2.erase();
        coin3.erase();

        Clear_UFO(x, y);

        /* collision detection */
        if (!boom_flag && collision_flag) {
            if (coin1.collide(x_c, y_c)) {
                coin1.init();
                score++;
                token_flag = true;
            }
            if (coin2.collide(x_c, y_c)) {
                coin2.init();
                score++;
                token_flag = true;
            }
            if (coin3.collide(x_c, y_c)) {
                coin3.init();
                score++;
                token_flag = true;
            }
            if (!token_flag) {
                vTaskResume(BoomScreenHandle);
                boom_flag = true;
                vTaskDelay(10);
                vTaskSuspend(gameplay_h);
                boom_flag = true;
            }
        }
        boom_flag = false;
        collision_flag = false;
        token_flag = false;

        xQueueReceive(orientation_q, &orientationQ_Buffer, 5);
        accel_value = orientationQ_Buffer;

        if (accel_value==0) {
            y++ ;
        } else if (accel_value==2) {
            y--;
        }
        if (y>58) {
            y=58;
        } else if (y<3) {
            y=3;
        }

        /* move obstacles */
        if (gameplay_timers[0] == 0) {
            x3--;
            if (ob1.isSlow()) ob1.shift();
            if (ob2.isSlow()) ob2.shift();
            if (ob3.isSlow()) ob3.shift();
            if (ob4.isSlow()) ob4.shift();
            coin1.shift();
            coin2.shift();
            coin3.shift();
        }
        if (gameplay_timers[1] == 0) {
            if (ob1.isMed()) ob1.shift();
            if (ob2.isMed()) ob2.shift();
            if (ob3.isMed()) ob3.shift();
            if (ob4.isMed()) ob4.shift();
        }
        if (gameplay_timers[2] == 0) {
            if (ob1.isFast()) ob1.shift();
            if (ob2.isFast()) ob2.shift();
            if (ob3.isFast()) ob3.shift();
            if (ob4.isFast()) ob4.shift();
        }

        /* recover obstacles */
        if (x3==0) {
            x3=64;
            y3= get_rand(5, 5);
            height3= get_rand(5, 7);
        }

        if (ob1.done()) ob1.init();
        if (ob2.done()) ob2.init();
        if (ob3.done()) ob3.init();
        if (ob4.done()) ob4.init();
        if (coin1.done()) coin1.init();
        if (coin2.done()) coin2.init();
        if (coin3.done()) coin3.init();

    }
}

Obstacle::Obstacle()
{
    init();
}

void Obstacle::setShape()
{
    uint32_t i = get_rand(4, 0);

    shape = rectangle;
    if (i == 0) {
        shape = circle;
    }
}

void Obstacle::setColour()
{
    uint32_t i = get_rand(sizeof(obstacle_colours) / sizeof(uint16_t), 0);

    c = obstacle_colours[i];
}

void Obstacle::setSpeed()
{
    uint32_t s = get_rand(64, 0), s2 = 0, s1 = 4;

    if (score > 64) {
        s2 = 31;
        s1 = 63;
    } else {
        s2 = 31 * score / 2048;
        s1 = 63 * score / 2048;
    }

    if (s <= s2) {
        speed = fast;
    } else if (s <= s1) {
        speed = medium;
    } else {
        speed = slow;
    }
}

void Obstacle::init()
{
    x = get_rand(16, 63);
    y = get_rand(60, 2);
    w = get_rand(4, 2);
    h = get_rand(7, 1);
    setShape();
    setColour();
    setSpeed();
}

void Obstacle::draw()
{
    rgb.drawObstacle(x, y, w, h, c, (shape == rectangle));
}

void Obstacle::erase()
{
    rgb.drawObstacle(x, y, w, h, BLACK, (shape == rectangle));
}

void Obstacle::shift()
{
    x--;
}

bool Obstacle::done()
{
    return (x <= 0);
}

bool Obstacle::isSlow()
{
    return (speed == slow);
}

bool Obstacle::isMed()
{
    return (speed == medium);
}

bool Obstacle::isFast()
{
    return (speed == fast);
}

#if 1
Coin::Coin()
{
    init();
}

void Coin::init()
{
    x = get_rand(64, 64);
    y = get_rand(60, 2);
}

void Coin::draw()
{
    rgb.drawPixel(x, y, GOLD);
}

void Coin::erase()
{
    rgb.drawPixel(x, y, BLACK);
}

void Coin::shift()
{
    x--;
}

bool Coin::done()
{
    return (x <= 0);
}

bool Coin::collide(uint16_t x_collide, uint16_t y_collide)
{
    return (x == x_collide) && (y == y_collide);
}
#endif
