#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem_dect_phy.h>
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/hwinfo.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>

#include <stdlib.h>

/*Heder files */
#include "hello.h"
#include "ping.h"
#include "core.h"
#include "utils.h"
#include "perf.h"
#include "perf_shell.h"
/*====================================*/




LOG_MODULE_REGISTER(shell, LOG_LEVEL_INF);

/* ======================= Common thread stack size ======================= */

#define STACK_SIZE 4096

/* ======================= HELLO worker thread ============================ */

K_THREAD_STACK_DEFINE(hello_stack, STACK_SIZE);
static struct k_thread hello_thread;

atomic_t hello_running = ATOMIC_INIT(0);
static char hello_mode = 't';   // 't' or 'r'

/* --- PING worker thread --- */

static uint32_t ping_count_cfg = 0;
static uint32_t hello_tx_count_cfg = 0;   /* 0 = use CONFIG_TX_TRANSMISSIONS */




void hello_worker(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    LOG_INF("HELLO thread started, mode=%c", hello_mode);

    if (hello_mode == 't') {
        /* One-shot TX: send up to hello_tx_count_cfg, then return */
        hello_start_tx(hello_tx_count_cfg);
    } else if (hello_mode == 'r') {
        /* RX: runs as long as hello_running == 1 inside hello_start_rx() */
        hello_start_rx();
    }

    /* After TX (or RX exiting due to mac stop), mark as stopped */
    atomic_set(&hello_running, 0);
    LOG_INF("HELLO thread stopped");
}


/* ======================= PING worker thread ============================= */


/* ======================= LED handling ================================== */

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec leds[] = {
#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
    GPIO_DT_SPEC_GET(LED0_NODE, gpios),
#endif
#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
    GPIO_DT_SPEC_GET(LED1_NODE, gpios),
#endif
#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
    GPIO_DT_SPEC_GET(LED2_NODE, gpios),
#endif
#if DT_NODE_HAS_STATUS(LED3_NODE, okay)
    GPIO_DT_SPEC_GET(LED3_NODE, gpios),
#endif
};

static int leds_init(void)
{
    int ret = 0;

    for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
        if (!device_is_ready(leds[i].port)) {
            LOG_ERR("LED%u device not ready", (unsigned int)(i + 1));
            ret = -ENODEV;
            continue;
        }

        int err = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
        if (err) {
            LOG_ERR("LED%u config failed (err=%d)", (unsigned int)(i + 1), err);
            ret = err;
        }
    }

    return ret;
}

static int led_set(uint8_t led_idx, bool on)
{
    if (led_idx == 0 || led_idx > ARRAY_SIZE(leds)) {
        return -EINVAL;
    }

    const struct gpio_dt_spec *l = &leds[led_idx - 1];

    return gpio_pin_set_dt(l, on ? 1 : 0);
}

/* ======================= LED shell commands ============================= */

static int cmd_led_on(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_print(shell, "Usage: hdect led on <1-4>");
        return -EINVAL;
    }

    uint8_t led = (uint8_t)strtoul(argv[1], NULL, 10);
    int err = led_set(led, true);

    if (err) {
        shell_print(shell, "Failed to turn LED%u ON (err=%d)", led, err);
    } else {
        shell_print(shell, "hdect: LED%u ON", led);
    }

    return err;
}

static int cmd_led_off(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_print(shell, "Usage: hdect led off <1-4>");
        return -EINVAL;
    }

    uint8_t led = (uint8_t)strtoul(argv[1], NULL, 10);
    int err = led_set(led, false);

    if (err) {
        shell_print(shell, "Failed to turn LED%u OFF (err=%d)", led, err);
    } else {
        shell_print(shell, "hdect: LED%u OFF", led);
    }

    return err;
}

/* ======================= HELLO shell commands =========================== */

static int cmd_hello_start(const struct shell *shell, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_print(shell,
                    "Usage: hdect mac start <t|r> [count]\n"
                    "  t = transmit only (optional count)\n"
                    "  r = receive only");
        return -EINVAL;
    }

    char mode = argv[1][0];

    if (mode != 't' && mode != 'r') {
        shell_print(shell, "Invalid mode. Use t or r");
        return -EINVAL;
    }

    if (atomic_get(&hello_running)) {
        shell_print(shell, "HELLO already running!");
        return -EBUSY;
    }

    /* Default: 0 = use CONFIG_TX_TRANSMISSIONS */
    hello_tx_count_cfg = 0;

    if (mode == 't') {
        if (argc >= 3) {
            hello_tx_count_cfg = (uint32_t)strtoul(argv[2], NULL, 10);
        }
    }

    hello_mode = mode;
    atomic_set(&hello_running, 1);

    k_thread_create(&hello_thread,
                    hello_stack,
                    STACK_SIZE,
                    hello_worker,
                    NULL, NULL, NULL,
                    5, 0, K_NO_WAIT);

    if (mode == 't') {
        if (hello_tx_count_cfg > 0) {
            shell_print(shell, "HELLO TX started, count=%u", hello_tx_count_cfg);
        } else {
            shell_print(shell, "HELLO TX started, using CONFIG_TX_TRANSMISSIONS");
        }
    } else {
        shell_print(shell, "HELLO started in RX mode");
    }

    return 0;
}


static int cmd_hello_stop(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    if (!atomic_get(&hello_running)) {
        shell_print(shell, "HELLO already stopped");
        return 0;
    }

    atomic_set(&hello_running, 0);
    shell_print(shell, "HELLO stopping...");

    return 0;
}

/* Forward declaration from hello.c */
int mac_send_text(const char *msg);

static int cmd_mac_mes(const struct shell *shell, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_error(shell, "Usage: hdect mac mes <text>");
        return -EINVAL;
    }

    const char *msg = argv[1];   /* text in quotes: "this is my text" */

    int err = mac_send_text(msg);
    if (err) {
        shell_error(shell, "MAC send failed: %d", err);
    } else {
        shell_print(shell, "MAC message sent: %s", msg);
    }

    return err;
}

/* ======================= PING shell commands ============================ */


/* -------- PING shell commands -------- */

static int cmd_ping_start(const struct shell *shell, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_print(shell, "Usage: hdect ping start <c|s> [count]");
        return -EINVAL;
    }

    char mode = argv[1][0];

    if (mode == 'c') {
        uint32_t count = 5;
        if (argc >= 3) {
            count = atoi(argv[2]);
        }

        int err = ping_start_client(count);
        if (err == 0)
            shell_print(shell, "PING CLIENT started");
        else
            shell_print(shell, "PING CLIENT failed");
    }
    else if (mode == 's') {
        int err = ping_start_server();
        if (err == 0)
            shell_print(shell, "PING SERVER started");
        else
            shell_print(shell, "PING SERVER failed");
    }
    else {
        shell_print(shell, "ERROR: mode must be c or s");
        return -EINVAL;
    }

    return 0;
}

static int cmd_ping_stop(const struct shell *shell, size_t argc, char **argv)
{
    ping_stop();
    shell_print(shell, "PING STOPPED");
    return 0;
}


/*Get Configureation */
static int cmd_cfg_show(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    struct hs_config cfg;
    int err = hs_core_get_config(&cfg);

    if (err) {
        shell_error(shell, "Failed to get config: %d", err);
        return err;
    }

    shell_print(shell, "=== HS-DECT configuration ===");
    shell_print(shell, "Device ID:        %u",        cfg.device_id);
    shell_print(shell, "Network ID:       0x%08x",    cfg.network_id);
    shell_print(shell, "Carrier index:    %u",        cfg.carrier);
    shell_print(shell, "Band group idx:   %u",        cfg.band_group_idx);
    shell_print(shell, "MCS:              %u",        cfg.mcs);
    shell_print(shell, "TX power (dBm):   %d",        cfg.tx_power);

    return 0;
}

/*Get Configureation */
static int cmd_cfg_set(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 3) {
        shell_print(shell,
            "Usage:\n"
            "  hdect cfg set carrier <1657..1677 odd>\n"
            "  hdect cfg set mcs     <%d..%d>\n"
            "  hdect cfg set txpwr   <%d..%d>",
            HS_MCS_MIN, HS_MCS_MAX,
            HS_TXPWR_MIN_DBM, HS_TXPWR_MAX_DBM);
        return -EINVAL;
    }

    const char *field   = argv[1];
    const char *val_str = argv[2];
    long v = strtol(val_str, NULL, 10);

    struct hs_config cfg;
    int err = hs_core_get_config(&cfg);
    if (err) {
        shell_error(shell, "Failed to get config: %d", err);
        return err;
    }

    if (strcmp(field, "carrier") == 0) {
        if (v < HS_CARRIER_MIN || v > HS_CARRIER_MAX || (v % 2) == 0) {
            shell_error(shell,
                        "Invalid carrier %ld. Must be odd in [%d..%d]",
                        v, HS_CARRIER_MIN, HS_CARRIER_MAX);
            return -EINVAL;
        }
        cfg.carrier = (uint16_t)v;

    } else if (strcmp(field, "mcs") == 0) {
        if (v < HS_MCS_MIN || v > HS_MCS_MAX) {
            shell_error(shell,
                        "Invalid MCS %ld. Must be [%d..%d]",
                        v, HS_MCS_MIN, HS_MCS_MAX);
            return -EINVAL;
        }
        cfg.mcs = (uint8_t)v;

    } else if (strcmp(field, "txpwr") == 0) {
        if (v < HS_TXPWR_MIN_DBM || v > HS_TXPWR_MAX_DBM) {
            shell_error(shell,
                        "Invalid TX power %ld dBm. Must be [%d..%d] dBm",
                        v, HS_TXPWR_MIN_DBM, HS_TXPWR_MAX_DBM);
            return -EINVAL;
        }
        cfg.tx_power = (int8_t)v;

    } else {
        shell_error(shell, "Unknown field '%s'. Use carrier|mcs|txpwr", field);
        return -EINVAL;
    }

    /* Push to core / modem */
    err = hs_core_apply_radio(cfg.carrier, cfg.mcs, cfg.tx_power);
    if (err) {
        shell_error(shell, "Failed to apply radio config: %d", err);
        return err;
    }

    shell_print(shell, "Config updated:");
    shell_print(shell, "  carrier = %u", cfg.carrier);
    shell_print(shell, "  mcs     = %u", cfg.mcs);
    shell_print(shell, "  txpwr   = %d dBm", cfg.tx_power);

    return 0;
}

/* Register 'configuration ' subcommands */
/* Forward declarations */
static int cmd_cfg_show(const struct shell *shell, size_t argc, char **argv);
static int cmd_cfg_set(const struct shell *shell, size_t argc, char **argv);
/* --- CFG subcommands: hdect cfg <...> --- */

SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_cfg,
    SHELL_CMD(show, NULL,
              "Show HS-DECT config\n"
              "  hdect cfg show",
              cmd_cfg_show),

    SHELL_CMD(set,  NULL,
              "Set HS-DECT config\n"
              "  hdect cfg set carrier <1657..1677 odd>\n"
              "  hdect cfg set mcs     <0..4>\n"
              "  hdect cfg set txpwr   <-30..19>",
              cmd_cfg_set),

    SHELL_SUBCMD_SET_END
);


/* Register 'perf' subcommands */


SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_perf,
    SHELL_CMD(start, NULL, "Start perf measurement", cmd_hdect_perf_start),
    SHELL_CMD(stop,  NULL, "Stop perf measurement",  cmd_hdect_perf_stop),
    SHELL_CMD(reset, NULL, "Reset perf metrics",      cmd_hdect_perf_reset),
    SHELL_CMD(show,  NULL, "Show perf metrics",       cmd_hdect_perf_show),
    SHELL_SUBCMD_SET_END
);



/* Register 'ping' subcommands */

SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_ping,
    SHELL_CMD(start,  NULL, "Start ping client: hdect ping start [count]", cmd_ping_start),
    SHELL_CMD(stop,   NULL, "Stop ping client/server",                     cmd_ping_stop),
    SHELL_SUBCMD_SET_END
);


/* ======================= Shell command trees ============================ */


SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_mac,
    SHELL_CMD(start, NULL, "Start MAC-DECT app (t/r)", cmd_hello_start),
    SHELL_CMD(stop,  NULL, "Stop MAC-DECT app",       cmd_hello_stop),
    SHELL_CMD(mes,   NULL, "Send MAC text: hdect mac mes <text>", cmd_mac_mes),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect_led,
    SHELL_CMD(on,  NULL, "Turn LED on  <1-4>",  cmd_led_on),
    SHELL_CMD(off, NULL, "Turn LED off <1-4>",  cmd_led_off),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_hdect,
    SHELL_CMD(led,  &sub_hdect_led,  "LED control",       NULL),
    SHELL_CMD(mac,  &sub_hdect_mac,  "MAC-DECT control",  NULL),
    SHELL_CMD(ping, &sub_hdect_ping, "DECT ping utility", NULL),
    SHELL_CMD(cfg,  &sub_hdect_cfg,  "HS-DECT config",    NULL),
    SHELL_CMD(perf,     &sub_hdect_perf,     "Performance metrics", NULL),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(hdect, &sub_hdect, "HS-DECT commands", NULL);

/* ======================= Init LEDs at boot ============================== */

static int hs_shell_init(const struct device *dev)
{
    ARG_UNUSED(dev);
    return leds_init();
}

SYS_INIT(hs_shell_init, APPLICATION, 90);

/*end of the file */