#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "hello.h"
#include "hs_shell.h"
#include "core.h"


void main(void)
{
    printk("HS_DECT: main() started\n");
    printk("Thanks fo beeing with us in this project !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
   // hs_shell_init();   // <-- enable shell commands

    // Start hello DECT
    //hello_start();
	//hello_start_ondemond();
    /*
    while (1) {
        k_sleep(K_SECONDS(1));
    } 
    
    
    */
    
}
