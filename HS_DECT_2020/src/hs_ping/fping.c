#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/sys/crc.h>
#include "hs_shell.h"
#include "fping.h"
#include "core.h"
#include <stdbool.h> 
#include <zephyr/sys/crc.h>
#include <stdint.h>
#include <stddef.h>
extern struct k_sem operation_sem;
LOG_MODULE_REGISTER(fping, LOG_LEVEL_INF);

/* --- CONFIG --- */
#define PING_BUF_SIZE 64
#define PING_TIMEOUT_MS 50

/* --- STATE --- */
static atomic_t fping_running = ATOMIC_INIT(0);
static uint32_t fping_expected_seq = 0;
static uint32_t fping_count_cfg = 0;
static uint8_t last_rx_buf[2048];
static size_t  last_rx_len;

static uint64_t last_tx_time_ms;
static volatile bool exit_flag = false;
/* ================================================
 *                 FPING PROTOCOL
 * ================================================ */

#define FPING_MAX_CHUNK   200   /* keep small enough for your DECT payload */
#define FPING_MAGIC       0x46504E47u /* 'FPNG' */

enum fping_type {
    FPING_BEGIN = 1,
    FPING_CHUNK = 2,
    FPING_END   = 3,
};

struct __packed fping_begin_pkt {
    uint32_t magic;
    uint8_t  type;          /* FPING_BEGIN */
    uint8_t  rsv0;
    uint16_t rsv1;
    uint32_t total_len;
    uint32_t crc32_expected;
};

struct __packed fping_chunk_hdr {
    uint32_t magic;
    uint8_t  type;          /* FPING_CHUNK */
    uint8_t  seq;
    uint16_t data_len;
    uint32_t offset;
    /* followed by data[data_len] */
};

struct __packed fping_end_pkt {
    uint32_t magic;
    uint8_t  type;          /* FPING_END */
    uint8_t  rsv0;
    uint16_t rsv1;
};

static struct {
    bool     active;
    uint32_t expected_len;
    uint32_t expected_crc;
    uint32_t rxlen;
    uint32_t rx_crc;  
    uint32_t rx_total;    /* running CRC */
} fping_srv;

static void fping_fill_deterministic(uint8_t *dst, size_t len, uint32_t offset)
{
    /* deterministic pattern: byte = (offset + i) & 0xFF */
    for (size_t i = 0; i < len; i++) {
        dst[i] = (uint8_t)((offset + (uint32_t)i) & 0xFF);
    }
}


/* Local IEEE CRC32 (Ethernet/ZIP): poly 0xEDB88320 */
static uint32_t hs_crc32_ieee_update(uint32_t crc, const uint8_t *data, size_t len)
{
    crc = ~crc;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}
extern int core_get_last_rx(uint8_t *dst, size_t max_len, size_t *out_len);

/* ================================================
 *                 SERVER THREAD
 * ================================================ */
/* You need a way to read the last received payload from core.
 * If your core already exposes something like: core_get_last_rx(...)
 * use it. If not, see section 2 below for a minimal core.c helper.
 */

static void fping_server_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    uint8_t rxbuf[300]; /* must be >= max packet size you expect */
    size_t rxlen = 0;

    memset(&fping_srv, 0, sizeof(fping_srv));

    while (!exit_flag) {

        /* arm RX */
        receive(0);
        k_sem_take(&operation_sem, K_FOREVER);

        if (core_get_last_rx(rxbuf, sizeof(rxbuf), &rxlen) != 0) {
            continue;
        }
        if (rxlen < 8) {
            continue;
        }

        uint32_t magic = *(uint32_t *)&rxbuf[0];
        uint8_t type = rxbuf[4];

        if (magic != FPING_MAGIC) {
            continue;
        }

        if (type == FPING_BEGIN) {
            if (rxlen < sizeof(struct fping_begin_pkt)) continue;
            const struct fping_begin_pkt *p = (const struct fping_begin_pkt *)rxbuf;

            fping_srv.active = true;
            fping_srv.expected_len = p->total_len;
            fping_srv.expected_crc = p->crc32_expected;
            fping_srv.rxlen = 0;
            fping_srv.rx_crc = 0;

            LOG_INF("FPING BEGIN: len=%u crc=0x%08x", p->total_len, p->crc32_expected);
        }
        else if (type == FPING_CHUNK) {
            if (!fping_srv.active) continue;
            if (rxlen < sizeof(struct fping_chunk_hdr)) continue;

            const struct fping_chunk_hdr *h = (const struct fping_chunk_hdr *)rxbuf;
            const uint8_t *data = rxbuf + sizeof(*h);
            int rxlen = (int)last_rx_len;
            if (rxlen < (int)sizeof(*h)) {
                continue; /* not enough for header */
            }
            size_t data_len = (size_t)rxlen - sizeof(*h);
            uint32_t n = h->data_len;

            if (sizeof(*h) + n > rxlen) continue;

            /* update running CRC + length */
           fping_srv.rx_crc = hs_crc32_ieee_update(fping_srv.rx_crc, data, data_len);


            fping_srv.rxlen += n;
        }
        else if (type == FPING_END) {
            if (!fping_srv.active) continue;

            bool ok_len = (fping_srv.rxlen == fping_srv.expected_len);
            bool ok_crc = (fping_srv.rx_crc == fping_srv.expected_crc);

            LOG_INF("FPING END: rxlen=%u exp_len=%u rx_crc=0x%08x exp_crc=0x%08x => %s",
                    fping_srv.rxlen, fping_srv.expected_len,
                    fping_srv.rx_crc, fping_srv.expected_crc,
                    (ok_len && ok_crc) ? "OK" : "FAIL");

            fping_srv.active = false;
        }
    }
}

/* ================================================
 *                 CLIENT THREAD
 * ================================================ */
static void fping_client_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    /* You can change these defaults later */
    const uint32_t total_len = 1024;   /* send 1 KB deterministic binary */
    const uint32_t chunk_len = FPING_MAX_CHUNK;

    /* compute expected CRC of deterministic data */
    uint32_t crc = 0;
    uint8_t tmp[FPING_MAX_CHUNK];
    for (uint32_t off = 0; off < total_len; off += chunk_len) {
        uint32_t n = total_len - off;
        if (n > chunk_len) n = chunk_len;
        fping_fill_deterministic(tmp, n, off);
       
    }

    /* SEND BEGIN */
    struct fping_begin_pkt begin = {
        .magic = FPING_MAGIC,
        .type = FPING_BEGIN,
        .total_len = total_len,
        .crc32_expected = crc,
    };

    transmit(0, &begin, sizeof(begin));
    k_sem_take(&operation_sem, K_FOREVER);

    /* SEND CHUNKS */
    uint8_t buf[sizeof(struct fping_chunk_hdr) + FPING_MAX_CHUNK];
    uint8_t seq = 0;

    for (uint32_t off = 0; off < total_len; off += chunk_len) {
        uint32_t n = total_len - off;
        if (n > chunk_len) n = chunk_len;

        struct fping_chunk_hdr *h = (struct fping_chunk_hdr *)buf;
        h->magic = FPING_MAGIC;
        h->type = FPING_CHUNK;
        h->seq = seq++;
        h->data_len = (uint16_t)n;
        h->offset = off;

        fping_fill_deterministic(buf + sizeof(*h), n, off);

        transmit(0, buf, sizeof(*h) + n);
        k_sem_take(&operation_sem, K_FOREVER);
    }

    /* SEND END */
    struct fping_end_pkt end = {
        .magic = FPING_MAGIC,
        .type = FPING_END,
    };

    transmit(0, &end, sizeof(end));
    k_sem_take(&operation_sem, K_FOREVER);

    LOG_INF("FPING client finished: len=%u crc=0x%08x", total_len, crc);
}

/* ================================================
 *             PUBLIC START/STOP FUNCTIONS
 * ================================================ */

K_THREAD_STACK_DEFINE(fping_stack, 2048);
static struct k_thread fping_thread;

int fping_server_start(void)
{
    if (atomic_get(&fping_running)) {
        LOG_WRN("FPING already running");
        return -EBUSY;
    }

    mac_rtt_set_enabled(false);   /* <--- no MAC RTT on server */

    atomic_set(&fping_running, 1);
    fping_expected_seq = 0;

    k_thread_create(&fping_thread, fping_stack, K_THREAD_STACK_SIZEOF(fping_stack),
                    fping_server_thread, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    return 0;
}


int fping_client_start(uint32_t count)
{
    fping_count_cfg = count;
    if (atomic_get(&fping_running)) {
        LOG_WRN("FPING already running");
        return -EBUSY;
    }

    atomic_set(&fping_running, 1);

    mac_rtt_set_enabled(true);    /* <--- MAC RTT only on client */

    k_thread_create(&fping_thread, fping_stack, K_THREAD_STACK_SIZEOF(fping_stack),
                    fping_client_thread, NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    return 0;
}


void fping_stop(void)
{
    if (!atomic_get(&fping_running)) {
        return;
    }

    LOG_INF("Stopping FPING...");
    atomic_set(&fping_running, 0);

}