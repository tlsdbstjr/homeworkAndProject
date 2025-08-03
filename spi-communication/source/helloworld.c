/*
 * rbpi_jstk.c: simple test application
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xil_io.h"

#define JSTK_AXI_IP_REG0 0x43C10000    // AXI address for joystick
#define JSTK_AXI_IP_REG1 0x43C10004    // AXI address + 4

#define NANO_AXI_IP_REG0 0x43C00000    // AXI address for Jetson Nano
#define NANO_AXI_IP_REG1 0x43C00004    // AXI address + 4

int main()
{
    // define GPIO object variables and user variables
    // GOIO is used for ZYNQ LEDs, and user variables are used for two AXI buses,
    //  one for the joystick and the other for Jetson nano
    XGpio gpio_obj;               // for ZYNQ LEDs
    unsigned int var;             // for AXI read/write

    // initialize platform
    init_platform();

    // Initialize GPIOs. Should match to the instance
    XGpio_Initialize(&gpio_obj, XPAR_AXI_GPIO_0_DEVICE_ID);

    // Direction Register (gpio_obj, channel=1, i/o: input is 1, output is 0)
    XGpio_SetDataDirection(&gpio_obj, 1, 0);

    print("we are up\n");

    // Initialize your local data here if you have to

    while (1) {// do forever
    // your main code here.
    // If you keep the same data format as the joystick data from the NANO command,
    // you don't need to change data format in this program.

      // read joystic data, then send to Nano
    	var = Xil_In32(JSTK_AXI_IP_REG0);
    	Xil_Out32(NANO_AXI_IP_REG0, var);
      // read Nano data, then send to joystick
    	var = Xil_In32(NANO_AXI_IP_REG1);
    	if((var&0xFFFF0000) != 0xFFFF0000) continue;
    	Xil_Out32(JSTK_AXI_IP_REG1, var & 1);
      // read Nano data, then send to ZYNQ LEDs
    	var >>= 8;
    	XGpio_DiscreteWrite(&gpio_obj, 1, var);
    }

    cleanup_platform();
    return 0;
}
