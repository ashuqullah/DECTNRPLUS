/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem_dect_phy.h>
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/hwinfo.h>
#include "hs_shell.h"
#include "core.h"
#include "hello.h"
#include "perf.h"
#define DATA_LEN_MAX 32

LOG_MODULE_REGISTER(hello, LOG_LEVEL_INF);

extern struct k_sem operation_sem;
/*End of the Session functions  */

int hello_start_tx(uint32_t max_tx)
{
    int err;
    uint32_t tx_handle = 0;
    uint32_t tx_counter_value = 0;
    uint8_t tx_buf[DATA_LEN_MAX];
    size_t tx_len;

    LOG_INF("DECT NR+ PHY TX sample started (max_tx = %u)", max_tx);

    err = dect_session_open();
    if (err) {
        return err;
    }

    /* Loop only while HELLO is running */
    while (atomic_get(&hello_running)) {

        LOG_INF("Transmitting %u", tx_counter_value);

        tx_len = sprintf(tx_buf, "Hello DECT! %u", tx_counter_value) + 1;
        err = transmit(tx_handle, tx_buf, tx_len);
        if (!err) {
            hs_perf_on_tx(tx_len);
        }
        if (err) {
            LOG_ERR("Transmission failed, err %d", err);
            break;
        }

        tx_counter_value++;

        /* Wait for TX operation to complete. */
        k_sem_take(&operation_sem, K_FOREVER);

        /* Optional: stop after max_tx transmissions if max_tx > 0 */
        if (max_tx && tx_counter_value >= max_tx) {
            LOG_INF("Reached maximum number of transmissions (%u)", max_tx);
            break;
        }

        /* 1 second between transmissions */
        k_sleep(K_SECONDS(1));
    }

    err = dect_session_close();
    return err;
}



int hello_start_rx(void)
{
    int err;
    uint32_t rx_handle = 1;
     size_t rx_len;
    LOG_INF("DECT NR+ PHY RX sample started");

    err = dect_session_open();
    if (err) {
        return err;
    }

    while (atomic_get(&hello_running)) {
        err = receive(rx_handle);
        if (err) {
            LOG_ERR("Reception failed, err %d", err);
            break;
        }

        /* Wait for RX operation to complete. */
        k_sem_take(&operation_sem, K_FOREVER);

       hs_perf_on_rx(rx_len);
    }

    err = dect_session_close();
    return err;
}


int mac_send_text(const char *msg)
{
    int err;
    uint32_t tx_handle = 0;
    uint8_t tx_buf[DATA_LEN_MAX];
    size_t tx_len;

    LOG_INF("MAC send text: '%s'", msg);

    tx_len = strlen(msg) + 1;
    if (tx_len > DATA_LEN_MAX) {
        tx_len = DATA_LEN_MAX;
    }
    memcpy(tx_buf, msg, tx_len);

    err = dect_session_open();
    if (err) {
        return err;
    }

    err = transmit(tx_handle, tx_buf, tx_len);
    if (err) {
        LOG_ERR("Transmission failed, err %d", err);
    } else {
        /* Wait for TX to complete */
        k_sem_take(&operation_sem, K_FOREVER);
        LOG_INF("MAC message sent");

        /* ✅ Tell perf module about this TX */
        hs_perf_on_tx(tx_len);
    }

    /* Close session (for now we keep your pattern) */
    err = dect_session_close();
    return err;
}
