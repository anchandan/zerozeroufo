/*
 * controller.hpp
 *
 *  Created on: Dec 4, 2018
 *      Author: ollie
 */

#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

#include "LPC17xx.h"
//#include "FreeRTOS.h"
#include "utilities.h"
#include "printf_lib.h"

#include "messages.hpp"

//#include "io.hpp"


void controller(void *p);


/* LabGpio_0.hpp ozhu */
extern SemaphoreHandle_t xSemaphore;

class LabGPIO
{
private:
    /**
     * port, pin and any other variables should be placed here.
     * NOTE: that the state of the pin should not be recorded here. The true
     *      source of that information is embedded in the hardware registers
     */
    uint8_t _port, _pin;
    LPC_GPIO_TypeDef *_gpio;
public:
    /**
     * You should not modify any hardware registers at this point
     * You should store the port and pin using the constructor.
     *
     * @param {uint8_t} pin  - pin number between 0 and 32
     */
    LabGPIO(uint8_t port, uint8_t pin);
    /**
     * Should alter the hardware registers to set the pin as an input
     */
    void setAsInput();
    /**
     * Should alter the hardware registers to set the pin as an input
     */
    void setAsOutput();
    /**
     * Should alter the set the direction output or input depending on the input.
     *
     * @param {bool} output - true => output, false => set pin to input
     */
    void setDirection(bool output);
    /**
     * Should alter the hardware registers to set the pin as high
     */
    void setHigh();
    /**
     * Should alter the hardware registers to set the pin as low
     */
    void setLow();
    /**
     * Should alter the hardware registers to set the pin as low
     *
     * @param {bool} high - true => pin high, false => pin low
     */
    void set(bool high);
    /**
     * Should return the state of the pin (input or output, doesn't matter)
     *
     * @return {bool} level of pin high => true, low => false
     */
    bool getLevel();
    ~LabGPIO();
};

void vControlLED( void * pvParameters );
void vReadSwitch( void * pvParameters );

class toggle_led : public scheduler_task
{
    public:
        toggle_led(uint32_t external);
        bool run(void *p);      ///< We run our code here
    private:
        uint32_t _ext;
};


class read_switch : public scheduler_task
{
    public:
        read_switch(uint32_t external);
        bool run(void *p);      ///< We run our code here
    private:
        uint32_t _ext;
};

/* LabInt.hpp ozhu */
typedef void (*IsrPointer)(void);

extern IsrPointer global_pin_isr_map[2][32];

typedef struct {
    __I  uint32_t IntStatR;
    __I  uint32_t IntStatF;
    __O  uint32_t IntClr;
    __IO uint32_t IntEnR;
    __IO uint32_t IntEnF;
} LabGpioInt;

enum InterruptCondition
{
    kRisingEdge,
    kFallingEdge,
    kBothEdges,
};

class LabGpioInterrupts
{
 public:
    /**
     * Optional: LabGpioInterrupts could be a singleton class, meaning, only one instance can exist at a time.
     * Look up how to implement this. It is best to not allocate memory in the constructor and leave complex
     * code to the Initialize() that you call in your main()
     */
    LabGpioInterrupts();

    /**
     * This should configure NVIC to notice EINT3 IRQs; use NVIC_EnableIRQ()
     */
    void Initialize();

    /**
     * This handler should place a function pointer within the lookup table for the HandleInterrupt() to find.
     *
     * @param[in] port         specify the GPIO port, and 1st dimension of the lookup matrix
     * @param[in] pin          specify the GPIO pin to assign an ISR to, and 2nd dimension of the lookup matrix
     * @param[in] pin_isr      function to run when the interrupt event occurs
     * @param[in] condition    condition for the interrupt to occur on. RISING, FALLING or BOTH edges.
     * @return should return true if valid ports, pins, isrs were supplied and pin isr insertion was successful
     */
    bool AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition);

    /**
     * This function is invoked by the CPU (through Eint3Handler) asynchronously when a Port/Pin
     * interrupt occurs. This function is where you will check the Port status, such as IO0IntStatF,
     * and then invoke the user's registered callback and find the entry in your lookup table.
     *
     * VERY IMPORTANT!
     *  - Be sure to clear the interrupt flag that caused this interrupt, or this function will be called
     *    repetitively and lock your system.
     *  - NOTE that your code needs to be able to handle two GPIO interrupts occurring at the same time.
     */
    void HandleInterrupt();

 private:
    /**
     * Allocate a lookup table matrix here of function pointers (avoid dynamic allocation)
     * Upon AttachInterruptHandler(), you will store the user's function callback
     * Upon the EINT3 interrupt, you will find out which callback to invoke based on Port/Pin status.
     */
    IsrPointer pin_isr_map[2][32];
};

void user_toggle_cb(void);
void Eint3Handler(void);

extern SemaphoreHandle_t xSemaphore;
extern LabGpioInterrupts gpio_interrupt;



#endif /* CONTROLLER_HPP_ */
