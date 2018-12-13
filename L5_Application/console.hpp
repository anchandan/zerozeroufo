/*
 * console.hpp
 *
 *  Created on: Dec 1, 2018
 */

#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include "LPC17xx.h"
#include "FreeRTOS.h"
#if 1
#include "task.h"
#endif
#include "printf_lib.h"
#include <time.h>
#include "utilities.h"

#include "RGB.hpp"
#include "GFX.hpp"

#include "messages.hpp"
#include "semphr.h"
//#include "acceleration_sensor.hpp"
//#include "io.hpp"

/* Colors */
#define WHITE   0xFFFF
#define GRAY    0x000F
#define MAGENTA 0xF81F
#define RED     0xF800
#define GREEN   0x0202
#define CYAN    0x01FF
#define VIOLET  0xFFE0
#define BLUE    0x85DD
#define PINK    0xFF00
#define BLACK   0x0000
//#define YELLOW


extern RGB rgb;
extern SemaphoreHandle_t update_display_semphr;
extern QueueHandle_t orientation_q;
extern QueueHandle_t control_q;


void timer0_ISR(void);
void update_display_task(void *p);
void receive_message(void *p);
void start_screen(void *p);
void console(void *p);

#if 1
void RGB_BoomScreen(void *p);
void gameplay(void *p);

extern TaskHandle_t gameplay_h;
extern TaskHandle_t start_h;
extern TaskHandle_t BoomScreenHandle;
#endif

class Obstacle
{
private:
    uint16_t x, y, w, h, c;
    uint32_t speed;
    uint16_t obstacle_colours[7] = { GRAY, MAGENTA, RED, GREEN, CYAN, BLUE, PINK };

    uint16_t setColour();
public:
    Obstacle();
    ~Obstacle();

    void init();
    void draw();
    void erase();
    void shift();
    uint16_t pos();
};


#endif /* CONSOLE_HPP_ */
