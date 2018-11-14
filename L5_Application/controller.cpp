#include "LabQueue.hpp"
#include "LabAdc.hpp"
#include "acceleration_sensor.hpp"
#include "io.hpp"
#include "wireless.h"

#ifdef CONTROLLER

void controller(void *p) /* HIGH priority */
{
    //orientation_t orientation = invalid;
    int x, y, z;
    //uint8_t device = 0x39;  /* 0x38 */

    //TODO call initialization function
    lpc_pconp(pconp_i2c2, true);
    lpc_pclk(pclk_i2c2, clkdiv_4);

    LPC_PINCON->PINSEL0 &= ~(0x3 << 20); /* P0.10 */
    LPC_PINCON->PINSEL0 |= (0x2 << 20);
    LPC_PINCON->PINSEL0 &= ~(0x3 << 22); /* P0.11 */
    LPC_PINCON->PINSEL0 |= (0x2 << 22);

    LPC_PINCON->PINMODE0 &= ~(0x3 << 20);
    LPC_PINCON->PINMODE0 &= ~(0x3 << 22);
    LPC_PINCON->PINMODE0 |= (0x2 << 20);
    LPC_PINCON->PINMODE0 |= (0x2 << 22);
    LPC_PINCON->PINMODE_OD0 |= (0x1 << 10);
    LPC_PINCON->PINMODE_OD0 |= (0x1 << 11);
    //LPC_PINCON->PINMODE_OD0 &= ~(0x1 << 10);
    //LPC_PINCON->PINMODE_OD0 &= ~(0x1 << 11);

    LPC_I2C2->I2CONSET = 0x20;

    while (1) {
        // This xQueueSend() will internally switch context over to the "consumer" task
        // because it is higher priority than this "producer" task
        // Then, when the consumer task sleeps, we will resume out of xQueueSend()
        // and go over to the next line
        x = AS.getX();
        y = AS.getY();
        z = AS.getZ();
        orientation = get_orientation(x, y, z);

        u0_dbg_printf("sending orientation\n");
        //xQueueSend(q, &orientation, 1000);
        u0_dbg_printf("sent orientation\n");

        //orientation = up;
        wireless_send(addr, mesh_pkt_nack, (const void *)&x, sizeof(x), 0);
        vTaskDelay(20);
    }
#if 0
    orientation_t orientation = invalid;
    while (1) {
        xQueueReceive(q, &orientation, portMAX_DELAY);
        u0_dbg_printf("received orientation %s\n", print_orientation(orientation));
        delay_ms(20);
    }
#endif
}

#endif /* CONTROLLER */
