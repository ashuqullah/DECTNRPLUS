#ifndef HS_PING_H
#define HS_PING_H

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>

int ping_start_client(uint32_t count);
int ping_start_server(void);
void ping_stop(void);

#endif
