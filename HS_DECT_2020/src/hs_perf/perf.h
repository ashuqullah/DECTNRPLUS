#pragma once

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hs_perf_metrics {
    uint32_t tx_pkts;
    uint32_t rx_pkts;
    uint32_t tx_bytes;
    uint32_t rx_bytes;

    uint32_t start_ms;
    uint32_t end_ms;
    uint32_t duration_ms;  /* end - start while running */

    /* Optional latency stats (used when you have RTT) */
    uint32_t rtt_count;
    uint32_t rtt_min_ms;
    uint32_t rtt_max_ms;
    uint64_t rtt_sum_ms;   /* for average */
};

void hs_perf_reset(void);
void hs_perf_start(void);
void hs_perf_stop(void);
bool hs_perf_is_running(void);

/* Hooks you call from TX/RX paths */
void hs_perf_on_tx(size_t bytes);
void hs_perf_on_rx(size_t bytes);

/* Hook for RTT (e.g. from macsched ACK) */
void hs_perf_on_rtt(uint32_t rtt_ms);

/* Read current metrics snapshot */
void hs_perf_get(struct hs_perf_metrics *out);

#ifdef __cplusplus
}
#endif
