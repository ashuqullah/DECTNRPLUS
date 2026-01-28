#ifndef HELLO_H__
#define HELLO_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


int hello_start_tx(uint32_t tx_count);
int hello_start_rx(void);
int mac_send_text(const char *msg);

extern atomic_t hello_running;


#endif /* HS_HELLO_H__ */
