#include "controller.hpp"
//#include "LabAdc.hpp"
#include "acceleration_sensor.hpp"
#include "io.hpp"
#include "wireless.h"

/* LabGpio_0.cpp ozhu */
/* TODO locking */
static uint32_t switch_internal = 0;
static uint32_t switch_external = 0;
#if 0
static uint8_t port_external = 1;
#else
static uint8_t port_external = 2;
#endif

toggle_led::toggle_led(uint32_t external) :
        scheduler_task("toggle_led", 4 * 512, PRIORITY_HIGH)
{
    _ext = external;
}

bool toggle_led::run(void *p)
{
    vControlLED((void *)_ext);
    return true;
}

read_switch::read_switch(uint32_t external) :
        scheduler_task("read_switch", 4 * 512, PRIORITY_HIGH)
{
    _ext = external;
}

bool read_switch::run(void *p)
{
    vReadSwitch((void *)_ext);
    return true;
}

void vControlLED( void * pvParameters )
{
    /* Get Parameter */
    uint32_t external = (uint32_t)(pvParameters);
    /* Define Constants Here */
    static const uint8_t LED_PIN_INTERNAL = 0;
    static const uint8_t LED_PIN_EXTERNAL_1 = 29;
    static const uint8_t LED_PIN_EXTERNAL_2 = 6;
    /* Define Local Variables and Objects */
    uint8_t port = 1, pin;
#if 0
    volatile uint32_t *switch_flag;
#endif
    if (external) {
        if (port_external == 2) {
            port = port_external;
            pin = LED_PIN_EXTERNAL_2;
        } else {
            pin = LED_PIN_EXTERNAL_1;
        }
#if 0
        switch_flag = &switch_external;
#endif
    } else {
        pin = LED_PIN_INTERNAL;
#if 0
        switch_flag = &switch_internal;
#endif
    }
    /* Initialization Code */
    LabGPIO gpio = LabGPIO(port, pin);
    gpio.setAsOutput();

    while(1) {
#if 0
        while (!(*switch_flag));
        *switch_flag = 0;
        gpio.set(!gpio.getLevel());
#else
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY)) {
            gpio.set(!gpio.getLevel());
        }
#endif
    }
    /* Only necessary if above loop has a condition */
    //xTaskDelete(NULL);
}

#if 0
void vReadSwitch( void * pvParameters )
{
    /* Get Parameter */
    uint32_t external = (uint32_t)(pvParameters);
    /* Define Constants Here */
    static const uint8_t SW_PIN_INTERNAL = 9;
    static const uint8_t SW_PIN_EXTERNAL_1 = 28;
    static const uint8_t SW_PIN_EXTERNAL_2 = 7;
    /* Define Local Variables and Objects */
    uint8_t port = 1, pin;
    volatile uint32_t *switch_flag;
    if (external) {
        if (port_external == 2) {
            port = port_external;
            pin = SW_PIN_EXTERNAL_2;
        } else {
            pin = SW_PIN_EXTERNAL_1;
        }
        switch_flag = &switch_external;
    } else {
        pin = SW_PIN_INTERNAL;
        switch_flag = &switch_internal;
    }
    /* Initialization Code */
    LabGPIO gpio = LabGPIO(port, pin);
    gpio.setAsInput();

    while(1) {
        /* Insert Loop Code */
        if (gpio.getLevel()) {
            while (gpio.getLevel());
            *switch_flag = 1;
        }
    }
    /* Only necessary if above loop has a condition */
    //xTaskDelete(NULL);
}
#endif

LabGPIO::LabGPIO(uint8_t port, uint8_t pin)
{
    _port = port;
    _pin = pin;

    if (_port == 1) {
        _gpio = LPC_GPIO1;
    } else if (_port == 2) {
        _gpio = LPC_GPIO2;
    } else {
        _gpio = LPC_GPIO1;
    }
}

void LabGPIO::setAsInput()
{
    _gpio->FIODIR &= ~(1 << _pin);
}

void LabGPIO::setAsOutput()
{
    _gpio->FIODIR |= (1 << _pin);
}

void LabGPIO::setDirection(bool output)
{
    if (output) {
        setAsOutput();
    } else {
        setAsInput();
    }
}

void LabGPIO::setHigh()
{
    _gpio->FIOSET = (1 << _pin);
}

void LabGPIO::setLow()
{
    _gpio->FIOCLR = (1 << _pin);
}

void LabGPIO::set(bool high)
{
    if (high) {
        setHigh();
    } else {
        setLow();
    }
}

bool LabGPIO::getLevel()
{
    if (_gpio->FIOPIN & (1 << _pin)) {
        return true;
    } else {
        return false;
    }
}

/* LabInt.cpp ozhu */
#if 0
        // Register your callback for the EINT3
        isr_register(EINT3_IRQn, interrupt_callback);

        // Configure the registers to trigger Port2 interrupt (such as P2.0 rising edge)
        LPC_GPIO2->FIODIR &= ~(1 << 0);
        LPC_GPIOINT->IO2IntEnR |= (1 << 0);
        NVIC_EnableIRQ(EINT3_IRQn);
        while (1) {
            continue; // Empty loop just to test the interrupt
        }
#endif

        /**
         * Typedef a function pointer which will help in code readability
         * For example, with a function foo(), you can do this:
         * IsrPointer function_ptr = foo;
         * OR
         * IsrPointer function_ptr = &foo;
         */
SemaphoreHandle_t xSemaphore;


        /**
         * Unless you design Singleton class, we need a global instance of our class because
         * the asynchronous Eint3Handler() will need to invoke our C++ class instance callback
         * WARNING: You must use this same instance while testing your main()
         */
LabGpioInterrupts gpio_interrupt;

IsrPointer global_pin_isr_map[2][32] = { 0 };

LabGpioInterrupts::LabGpioInterrupts()
{
    ;
}

void
LabGpioInterrupts::Initialize()
{
    NVIC_EnableIRQ(EINT3_IRQn);
}

bool
LabGpioInterrupts::AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr,
        InterruptCondition condition)
{
    // Configure the registers to trigger Port2 interrupt (such as P2.0 rising edge)
    uint32_t port_index;
    LPC_GPIO_TypeDef *gpio;
    LabGpioInt *gpio_int;

    if (port == 0) {
        port_index = 0;
        gpio = LPC_GPIO0;
        gpio_int = (LabGpioInt *) &(LPC_GPIOINT->IO0IntStatR);
    } else if (port == 2) {
        port_index = 1;
        gpio = LPC_GPIO2;
        gpio_int = (LabGpioInt *) &(LPC_GPIOINT->IO2IntStatR);
    } else {
        u0_dbg_printf("port %d out of range [0-4]", port);
        return false;
    }

    if (pin >= 32) {
        u0_dbg_printf("pin %d out of range [0-31]", pin);
        return false;
    }
    /* set input pin */
    gpio->FIODIR &= ~(1 << pin);

    if (condition == kRisingEdge) {
        gpio_int->IntEnR |= (1 << pin);
    } else if (condition == kFallingEdge) {
        gpio_int->IntEnF |= (1 << pin);
    } else if (condition == kBothEdges) {
        gpio_int->IntEnR |= (1 << pin);
        gpio_int->IntEnF |= (1 << pin);
    } else {
        u0_dbg_printf("invalid interrupt condition %d", condition);
        return false;
    }

    if (pin_isr_map[port_index][pin] == NULL) {
        pin_isr_map[port_index][pin] = pin_isr;
    } else {
        u0_dbg_printf("overwrite pin isr map %p", (void *)pin_isr_map[port_index][pin]);
        pin_isr_map[port_index][pin] = pin_isr;
        return false;
    }

    return true;
}

void
LabGpioInterrupts::HandleInterrupt()
{
    uint32_t port_index, pin;

    u0_dbg_printf("%x;0:%x,2:%x\n", LPC_GPIOINT->IntStatus, LPC_GPIOINT->IO0IntStatR,
                LPC_GPIOINT->IO2IntStatR);
    if (LPC_GPIOINT->IntStatus & (1 << 0)) {
        port_index = 0;
        for (pin = 0; pin < 32; pin++) {
            if ((LPC_GPIOINT->IO0IntStatR | LPC_GPIOINT->IO0IntStatF) & (1 << pin)) {
                pin_isr_map[port_index][pin]();
                LPC_GPIOINT->IO0IntClr |= (1 << pin);
            }
        }
    }
    if (LPC_GPIOINT->IntStatus & (1 << 2)) {
        port_index = 1;
        for (pin = 0; pin < 32; pin++) {
            if ((LPC_GPIOINT->IO2IntStatR | LPC_GPIOINT->IO2IntStatF) & (1 << pin)) {
                pin_isr_map[port_index][pin]();
                LPC_GPIOINT->IO2IntClr |= (1 << pin);
            }
        }
    }
}

void interrupt_callback(void)
{
    // Clear the source of the EINT3 interrupt
    // Maybe uart0_puts() or blink an LED here to test your ISR
    static uint32_t count = 0;

    LPC_GPIOINT->IO2IntClr |= (1 << 0);
    switch (count % 4) {
    case 0:
        u0_dbg_printf("because the world is round\n");
        break;
    case 1:
        u0_dbg_printf("it turns me on\n");
        break;
    case 2:
        u0_dbg_printf("because the wind is high\n");
        break;
    case 3:
        u0_dbg_printf("it blows my mind\n");
        break;
    default:
        u0_dbg_printf("ERROR\n");
        break;
    }

    count++;
}

void user_toggle_cb(void)
{
  // This is where you will "send" a Semaphore that the vControlLED task is waiting on
    portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &pxHigherPriorityTaskWoken);
}


/* Since we have a C++ class handle an interrupt, we need to setup a C function delegate to invoke it
 * So here is the skeleton code that you can reference.
 * This function will simply delegate the interrupt handling to our C++ class
 * The CPU interrupt should be attached to this function through isr_register()
 */
void Eint3Handler(void)
{
    gpio_interrupt.HandleInterrupt();
}


/* ozhu */
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
#if 0
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
#endif
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

