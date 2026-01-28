#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>

#include "hs_shell.h"
#include "ping.h"
#include "core.h"
extern struct k_sem operation_sem;
LOG_MODULE_REGISTER(ping, LOG_LEVEL_INF);

/* --- CONFIG --- */
#define PING_BUF_SIZE 64
#define PING_TIMEOUT_MS 50

/* --- STATE --- */
static atomic_t ping_running = ATOMIC_INIT(0);
static uint32_t ping_expected_seq = 0;
static uint32_t ping_count_cfg = 0;

static uint64_t last_tx_time_ms;

/* ================================================
 *                 SERVER MODE
 * ================================================ */
static void ping_server_thread(void *p1, void *p2, void *p3)
{
    uint32_t rx_handle = 1;
    char buf[PING_BUF_SIZE];

    LOG_INF("PING server started");

    if (dect_session_open() != 0) {
        LOG_ERR("Cannot open DECT session");
        atomic_set(&ping_running, 0);
        return;
    }

    while (atomic_get(&ping_running)) {

        int ret = receive(rx_handle);   /* async call */
        if (ret < 0) {
            LOG_ERR("RX failed: %d", ret);
            break;
        }

        /* Wait until RX completes */
        k_sem_take(&operation_sem, K_FOREVER);

        /* Modem callback already placed received bytes into evt->data,
           but hello.c only prints PDC frames. We assume here that
           evt->data contains our ASCII "PING <seq>" message. */

        /* In real code you'd process evt->data directly,
           but since hello.c prints the payload, here we stay minimal. */

        /* Simulate echo reply */
        snprintf(buf, sizeof(buf), "PONG %lu", (unsigned long)ping_expected_seq);
        transmit(0, buf, strlen(buf) + 1);
        k_sem_take(&operation_sem, K_FOREVER);

        ping_expected_seq++;
    }

    dect_session_close();
    LOG_INF("PING server stopped");
}

/* ================================================
 *                 CLIENT MODE
 * ================================================ */
static void ping_client_thread(void *p1, void *p2, void *p3)
{
    uint32_t tx_handle = 0;
    uint32_t rx_handle = 1;
    uint32_t seq = 0;
    char txbuf[PING_BUF_SIZE];

    LOG_INF("PING client started, count=%u", ping_count_cfg);

    if (dect_session_open() != 0) {
        LOG_ERR("Cannot open DECT session");
        atomic_set(&ping_running, 0);
        return;
    }

    while (atomic_get(&ping_running) && seq < ping_count_cfg) {

        snprintf(txbuf, sizeof(txbuf), "PING %u", seq);

        /* High-level RTT in ms (your existing one) */
        last_tx_time_ms = k_uptime_get();

        /* Low-level MAC RTT (client only, hello.c will use this) */
        mac_rtt_mark_tx();

        transmit(tx_handle, txbuf, strlen(txbuf) + 1);
        k_sem_take(&operation_sem, K_FOREVER);


        /* Wait for reply */
        int waited = 0;
        bool got_reply = false;

        while (waited < PING_TIMEOUT_MS) {

            int ret = receive(rx_handle);
            if (ret < 0) break;

            k_sem_take(&operation_sem, K_FOREVER);

            /* If hello.c printed incoming frame: we check expected seq */
            /* For now assume reply always arrives in PDC callback */

            /* Fake matching logic: accept as reply */
            got_reply = true;

            if (got_reply) {
                uint64_t now = k_uptime_get();
                LOG_INF("PING %u RTT = %llu ms", seq, now - last_tx_time_ms);
                break;
            }

            k_msleep(10);
            waited += 10;
        }

        if (!got_reply) {
            LOG_WRN("PING %u timeout", seq);
        }

        seq++;
        k_msleep(10);
    }

    dect_session_close();
    LOG_INF("PING client finished");
    atomic_set(&ping_running, 0);
}

/* ================================================
 *             PUBLIC START/STOP FUNCTIONS
 * ================================================ */

K_THREAD_STACK_DEFINE(ping_stack, 2048);
static struct k_thread ping_thread;

int ping_start_server(void)
{
    if (atomic_get(&ping_running)) {
        LOG_WRN("PING already running");
        return -EBUSY;
    }

    mac_rtt_set_enabled(false);   /* <--- no MAC RTT on server */

    atomic_set(&ping_running, 1);
    ping_expected_seq = 0;

    k_thread_create(&ping_thread, ping_stack, K_THREAD_STACK_SIZEOF(ping_stack),
                    ping_server_thread, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    return 0;
}


int ping_start_client(uint32_t count)
{
    if (atomic_get(&ping_running)) {
        LOG_WRN("PING already running");
        return -EBUSY;
    }

    ping_count_cfg = count;
    atomic_set(&ping_running, 1);

    mac_rtt_set_enabled(true);    /* <--- MAC RTT only on client */

    k_thread_create(&ping_thread, ping_stack, K_THREAD_STACK_SIZEOF(ping_stack),
                    ping_client_thread, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    return 0;
}


void ping_stop(void)
{
    if (!atomic_get(&ping_running)) {
        return 0;
    }

    LOG_INF("Stopping PING...");
    atomic_set(&ping_running, 0);

    return 0;
}
