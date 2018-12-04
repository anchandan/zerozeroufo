#include "controller.hpp"
//#include "LabAdc.hpp"
#include "acceleration_sensor.hpp"
#include "io.hpp"
#include "wireless.h"

void controller(void *p) /* HIGH priority */
{
#if 0
    orientation_t orientation = invalid;
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
        wireless_send(CONSOLE_NODE_ADDR, mesh_pkt_nack, (const void *)&x, sizeof(x), 0);
        vTaskDelay(20);
    }
#endif
#if 0
    orientation_t orientation = invalid;
    while (1) {
        xQueueReceive(q, &orientation, portMAX_DELAY);
        u0_dbg_printf("received orientation %s\n", print_orientation(orientation));
        delay_ms(20);
    }
#endif//ozhu
}

#if 0
typedef enum
{
    invalid,
    left,
    right,
    down,
    up,
}orientation_t;

QueueHandle_t consumer_queue;
#endif//ozhu

void producer(void *pv)
{
    uint32_t accel_orientation;
#if 0
    int x = 0, y = 0 , z = 0 ,orientation, count = 0, temp = 0;
#else
    int x = 0, orientation, count = 0;
#endif
    while(1)
    {
        x = AS.getX();
        //temp = abs(x);
        //if(temp > 100)
        //{
            if(x > 300)
            {
                accel_orientation = 2;
                //printf("up\n");
            }
            else if(x < -300)
            {
                accel_orientation = 0;
                //printf("down\n");
            }
            else if(x > -300 && x < 300)
            {
                accel_orientation = 1;
                //printf("stay\n");
            }
            //printf("%d\n",x);
        //}
        //y = AS.getY();
        //z = AS.getZ();
//        if(x < 0 && y > 0 && z > 0)
//        {
//            orientation = down;
//            printf("Sending orientation\r\n");
//            //xQueueSend(consumer_queue,&orientation,0);
//            printf("Orientation sent\r\n");
//        }
//        else if(x > 0 && y > 0 && z > 0)
//        {
//            printf("Sending orientation\r\n");
//            orientation = up;
//            //xQueueSend(consumer_queue,&orientation,0);
//            printf("Orientation sent\r\n");
//        }
//        else if(x > 0 && y < 0 && z > 0)
//        {
//            printf("Sending orientation\r\n");
//            orientation = left;
//            //xQueueSend(consumer_queue,&orientation,0);
//            printf("Orientation sent\r\n");
//        }
//        else if(x > 0 && y > 0 && z < y)
//        {
//            printf("Sending orientation\r\n");
//            orientation = right;
//            //xQueueSend(consumer_queue,&orientation,0);
//            printf("Orientation sent\r\n");
//        }
        //printf("%d,%d,%d\n",AS.getX(),AS.getY(),AS.getZ());
        //printf("%d\n",AS.getX());
        vTaskDelay(50);
    }
}

void wireless_transmit(void *pv)
{
    uint8_t addr = 105;
              //mesh_packet_t pkt;


    while (1) {
        wireless_send(addr, mesh_pkt_nack, &accel_orientation, sizeof(accel_orientation), 0);
        //count++;
        //wireless_send(addr, mesh_pkt_nack, "hello", 5, 0);
        //wireless_send(addr, mesh_pkt_ack, NULL, 0, 0);
        delay_ms(50);
    }
}

#if 0
void consumer(void *pv)
{
    int received_orientation;
    while(1)
    {
        xQueueReceive(consumer_queue,&received_orientation,portMAX_DELAY);
        if(received_orientation == up)
        printf("Orientation received:up\r\n");
        else if(received_orientation == down)
            printf("Orientation received:down\r\n");
        else if(received_orientation == right)
                    printf("Orientation received:right\r\n");
        else if(received_orientation == left)
                    printf("Orientation received:left\r\n");
    }
}
#endif//ozhu

#if 0
void adc_producer_task(void *pv)
{
    printf("a");
    int light_intensity,sum = 0;
    float avg = 0;
    int count = 1;
    LabADC producer_obj;
    producer_obj.ADCInitBurstMode();
    producer_obj.ADCSelectPin(producer_obj.k0_25);
    while(1)
    {
        if(count == 100)
        {
            //vTaskDelay(1000);
            printf("Sending:%f",avg);
            xQueueSend(producer_adc_queue,&avg,0);
            count = 1;
            avg = 0;
            sum = 0;
             //vTaskDelay(100);
        }
        else
        {
            light_intensity = producer_obj.ReadADCVoltageByChannel(2);
            //printf("Light intensity:%f\r\n",light_intensity);
            sum = sum+light_intensity;
            //printf("Intermediate avg:%f\r\n",avg);
            avg = sum/count;
            //printf("avg:%f,count:%d\r\n",avg,count);
            vTaskDelay(10);
        }
        count++;
//        printf("Average light intensity:%f\r\n",avg);
        //xQueueSend(producer_adc_queue,&avg,0);
        //printf("Sent data to queue\r\n");
  //      event_bits = xEventGroupSetBits(watchdog_handle,producer_bit);

    }
}
#endif//ozhu

#if 0
int write_file(char *filename,char *buffer)
{
    char *filename_argument = NULL;
    sprintf(filename_argument,"0:%s.txt",filename);
    printf("filename_argument:%s\nbuffer:%s",filename_argument,buffer);
    return Storage::write(filename_argument, buffer , strlen(buffer) , 0);
    //Storage::write("0:myfile.txt", "hello" , 5 , 0);
}

int append_file(char *filename,char *buffer)
{
    char *filename_argument = NULL;
    sprintf(filename_argument,"0:%s.txt",filename);
    return Storage::append(filename_argument, buffer , strlen(buffer) , 0);
    //Storage::write("0:myfile.txt", "hello" , 5 , 0);
}

void adc_consumer_task(void *pv)
{
    int write_result;
    rtc_t time;
    int count = 0,first = 0;
    char data_to_write_buffer[200];
    char *data_to_flash = NULL;
    float received_light_intensity;
    while(1)
    {
        //while(count <= 10)
        //{
            printf("Waiting for queue to receive\r\n");
            if(xQueueReceive(producer_adc_queue,&received_light_intensity,portMAX_DELAY))
            printf("Got data:%f\r\n",received_light_intensity);
#if 1
            time = rtc_gettime();
            sprintf(data_to_write_buffer,"%02d:%02d:%02d,%03f\0",time.hour,time.min,time.sec,received_light_intensity);
            printf("data_to_write_buffer:%s",data_to_write_buffer);

            //memset(data_to_write_buffer,0,sizeof(data_to_write_buffer));
            count++;
#endif
        //write_result = write_file("sensor.txt","ABC");
        //printf("write result:%d",write_result);
        //}
#if 0
        //strcat(data_to_write_buffer,'\0');
        if(first == 0)
        {
            write_result = write_file("sensor.txt",data_to_write_buffer);
            first = 1;
        }
        else
        {
            write_result = append_file("sensor.txt",data_to_write_buffer);
        }
        if(!write_result)
        {
            printf("Write not successful\r\n");
        }
#endif
        //data_to_flash = NULL;
        //count = 0;
        event_bits = xEventGroupSetBits(watchdog_handle,consumer_bit);
    }
}

void watchdog_task(void *pv)
{
    EventBits_t check_bits;
    watchdog_handle = xEventGroupCreate();
    int first = 0;
    while(1)
    {
        vTaskDelay(1000);
        check_bits = xEventGroupGetBits(watchdog_handle);
        printf("check_bits:%lu",check_bits);
#if 0
        if((check_bits & producer_bit) != 0)
        {
            if(first == 0)
            {
                write_file("stuck.txt","producer stuck\n");
            }
            else
            {
                append_file("stuck.txt","producer stuck\n");
            }
        }
        else if((check_bits & consumer_bit) != 0)
        {
            if(first == 0)
            {
                write_file("stuck.txt","consumer stuck\n");
            }
            else
            {
                append_file("stuck.txt","consumer stuck\n");
            }
        }
#endif
    }
}
#endif//ozhu

