#include "console.hpp"

RGB rgb;
SemaphoreHandle_t update_display_semphr;
QueueHandle_t orientation_q;
QueueHandle_t control_q;


void timer0_ISR(void)
{
#if 0
    portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;

    if((LPC_TIM0->IR & 0x01) == 0x01) { /* if MR0 interrupt */
        /* Clear MR0 interrupt flag */
        xSemaphoreGiveFromISR(update_display_semphr, &pxHigherPriorityTaskWoken);
        LPC_TIM0->IR |= (1 << 0);
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

void receive_msg(void *p)
{
    mesh_packet_t pkt;
    uint32_t opcode;
    uint32_t accel_value;

    while (1) {
        if (wireless_get_rx_pkt(&pkt, 50)) {
            wireless_deform_pkt(&pkt, 2,
                    &opcode, sizeof(opcode),
                    &accel_value, sizeof(accel_value));
            if (opcode == ZZU_DATA) {
                xQueueSend(orientation_q, &accel_value, 0);
            } else if (opcode == ZZU_CTRL) {
                xQueueSend(control_q, &accel_value, 0);
            } else {
                u0_dbg_printf("ERROR: invalid opcode %d\n", opcode);
            }
            u0_dbg_printf("xQueueSend here: %d \t\n", accel_value);
        }
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

void title_screen(void *p)
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
    while (1) {
        x = 7;
        y = 50;
        sd = 0;
        while (y > 25 && x <=58) {
            Draw_UFO_Start(x++, y--);

            //Clear_UFO(x,y);
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
}

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
