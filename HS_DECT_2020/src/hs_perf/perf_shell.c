#include <zephyr/shell/shell.h>
#include "perf.h"

int cmd_hdect_perf_start(const struct shell *shell,
                         size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    hs_perf_start();
    shell_print(shell, "perf: started");
    return 0;
}

int cmd_hdect_perf_stop(const struct shell *shell,
                        size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    hs_perf_stop();
    shell_print(shell, "perf: stopped");
    return 0;
}

int cmd_hdect_perf_reset(const struct shell *shell,
                         size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    hs_perf_reset();
    shell_print(shell, "perf: reset");
    return 0;
}

int cmd_hdect_perf_show(const struct shell *shell,
                        size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    struct hs_perf_metrics m;
    hs_perf_get(&m);

    float dur_s = (m.duration_ms > 0) ? (m.duration_ms / 1000.0f) : 0.0f;
    ARG_UNUSED(dur_s); /* if you don’t use it */

    shell_print(shell,
        "perf metrics:\n"
        "  running      : %s\n"
        "  duration_ms  : %u\n"
        "  tx_pkts      : %u\n"
        "  rx_pkts      : %u\n"
        "  tx_bytes     : %u\n"
        "  rx_bytes     : %u",
        hs_perf_is_running() ? "yes" : "no",
        m.duration_ms,
        m.tx_pkts,
        m.rx_pkts,
        m.tx_bytes,
        m.rx_bytes);

    if (m.duration_ms > 0) {
    /* bits per second = bytes * 8 / seconds
     * here we compute kbps with integer math:
     * kbps = (bytes * 8 * 1000) / duration_ms
     */
    uint32_t tx_kbps = (m.tx_bytes * 8U * 1000U) / (m.duration_ms ? m.duration_ms : 1U);
    uint32_t rx_kbps = (m.rx_bytes * 8U * 1000U) / (m.duration_ms ? m.duration_ms : 1U);

    /* and Mbps * 1000 to keep 3 decimals: Mbps = kbps / 1000.0
     * print as X.YYY using integer division & remainder
     */
    uint32_t tx_mbps_int  = tx_kbps / 1000U;
    uint32_t tx_mbps_frac = tx_kbps % 1000U;

    uint32_t rx_mbps_int  = rx_kbps / 1000U;
    uint32_t rx_mbps_frac = rx_kbps % 1000U;

    shell_print(shell,
        "  tx_throughput [Mbps]: %u.%03u\n"
        "  rx_throughput [Mbps]: %u.%03u",
        tx_mbps_int, tx_mbps_frac,
        rx_mbps_int, rx_mbps_frac);
}


    return 0;
}
