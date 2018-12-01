//#include "LabQueue.hpp"
//#include "LabAdc.hpp"
#include "acceleration_sensor.hpp"
#include "io.hpp"
#include "wireless.h"

#ifdef ZZU_CONSOLE

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
        vTaskDelay(20);
    }
}

#endif /* ZZU_CONSOLE */
