#include "perf.h"

#include <string.h>

static struct hs_perf_metrics g_perf;
static bool g_running;

void hs_perf_reset(void)
{
    memset(&g_perf, 0, sizeof(g_perf));
    g_running = false;
}

void hs_perf_start(void)
{
    hs_perf_reset();
    g_running = true;
    g_perf.start_ms = (uint32_t)k_uptime_get();
}

void hs_perf_stop(void)
{
    if (!g_running) {
        return;
    }
    g_running = false;
    g_perf.end_ms = (uint32_t)k_uptime_get();
    g_perf.duration_ms = g_perf.end_ms - g_perf.start_ms;
}

bool hs_perf_is_running(void)
{
    return g_running;
}

static void hs_perf_update_time(void)
{
    if (!g_running) {
        return;
    }
    uint32_t now_ms = (uint32_t)k_uptime_get();
    g_perf.end_ms = now_ms;
    g_perf.duration_ms = now_ms - g_perf.start_ms;
}

void hs_perf_on_tx(size_t bytes)
{
    if (!g_running) {
        return;
    }
    g_perf.tx_pkts++;
    g_perf.tx_bytes += (uint32_t)bytes;
    hs_perf_update_time();
}

void hs_perf_on_rx(size_t bytes)
{
    if (!g_running) {
        return;
    }
    g_perf.rx_pkts++;
    g_perf.rx_bytes += (uint32_t)bytes;
    hs_perf_update_time();
}

void hs_perf_on_rtt(uint32_t rtt_ms)
{
    if (!g_running) {
        return;
    }

    if (g_perf.rtt_count == 0) {
        g_perf.rtt_min_ms = rtt_ms;
        g_perf.rtt_max_ms = rtt_ms;
    } else {
        if (rtt_ms < g_perf.rtt_min_ms) {
            g_perf.rtt_min_ms = rtt_ms;
        }
        if (rtt_ms > g_perf.rtt_max_ms) {
            g_perf.rtt_max_ms = rtt_ms;
        }
    }

    g_perf.rtt_count++;
    g_perf.rtt_sum_ms += rtt_ms;
    hs_perf_update_time();
}

void hs_perf_get(struct hs_perf_metrics *out)
{
    if (!out) {
        return;
    }
    *out = g_perf;
}
