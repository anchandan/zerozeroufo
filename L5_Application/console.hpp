/*
 * console.hpp
 *
 *  Created on: Dec 1, 2018
 *      Author: ollie
 */

#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include "LPC17xx.h"
#include "FreeRTOS.h"
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
void receive_msg(void *p);
void title_screen(void *p);
void console(void *p);


#endif /* CONSOLE_HPP_ */
