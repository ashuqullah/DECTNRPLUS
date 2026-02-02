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
#include "core.h"
#include "perf.h"
#include "fping.h"
#include "ping.h"
LOG_MODULE_REGISTER(app);

BUILD_ASSERT(CONFIG_CARRIER, "Carrier must be configured according to local regulations");

#define DATA_LEN_MAX 32

static bool exit;
static uint16_t device_id;
static uint64_t modem_time;

static bool mac_rtt_enabled = false;
static uint64_t mac_last_tx_time_us = 0;
static uint32_t mac_last_tx_cyc;     // cycle counter at TX time


static bool mac_rtt_waiting_tx_time;
static int64_t mac_last_tx_modem_time_us;

static int    hs_carrier       = 1677;   /* default */
static int    hs_band_group    = 0;      /* what you print now */
static int    hs_mcs           = 1;
static int    hs_tx_power      = 13;
static uint32_t hs_network_id  = CONFIG_NETWORK_ID;
static uint16_t hs_device_id   = 0;      /* set after hwinfo_get_device_id() */
static uint8_t last_rx_buf[2048];
static size_t  last_rx_len;
/* ================= FILE TRANSFER (PING EXTENSION) ================= */

static atomic_t dect_busy = ATOMIC_INIT(0);

bool dect_try_lock(void)
{
    return atomic_cas(&dect_busy, 0, 1);
}

void dect_unlock(void)
{
    atomic_clear(&dect_busy);
}

/*Helpers */

void mac_rtt_set_enabled(bool enable)
{
    mac_rtt_enabled = enable;
    mac_rtt_waiting_tx_time = false;
}

/* Called from ping.c right before transmit() just to “arm” RTT measurement */
void mac_rtt_mark_tx(void)
{
    if (!mac_rtt_enabled) {
        return;
    }
    mac_rtt_waiting_tx_time = true;
}

/* Call this from the TX DONE / TX COMPLETE callback (modem event) */
static void mac_rtt_on_tx_done_modem_time(int64_t tx_time_us)
{
    if (!mac_rtt_enabled) {
        return;
    }
    if (!mac_rtt_waiting_tx_time) {
        return; /* ignore other TXs */
    }

    mac_last_tx_modem_time_us = tx_time_us;
    mac_rtt_waiting_tx_time = false;
}

/* Call this when you receive PONG in on_pdc() */
static void mac_rtt_on_pong_rx_modem_time(int64_t rx_time_us)
{
    if (!mac_rtt_enabled) {
        return;
    }

    int64_t rtt_us = rx_time_us - mac_last_tx_modem_time_us;

    LOG_INF("MAC RTT (client only): %lld us (%lld ms)",
            rtt_us, rtt_us / 1000);
}


void mac_rtt_on_rx(void)
{
    if (!mac_rtt_enabled) {
        return;
    }

    /* Read RX timestamp */
    uint32_t now_cyc  = k_cycle_get_32();

    /*
     * Compute delta in cycles.
     * Unsigned subtraction handles cycle wrap-around automatically.
     */
    uint32_t diff_cyc = now_cyc - mac_last_tx_cyc;

    /* Convert delta cycles → microseconds */
    uint32_t diff_us  = k_cyc_to_us_near32(diff_cyc);

    LOG_INF("MAC RTT (client only): %u us (%u ms)",
            diff_us, diff_us / 1000);
}

/* Header type 1, due to endianness the order is different than in the specification. */
struct phy_ctrl_field_common {
	uint32_t packet_length : 4;
	uint32_t packet_length_type : 1;
	uint32_t header_format : 3;
	uint32_t short_network_id : 8;
	uint32_t transmitter_id_hi : 8;
	uint32_t transmitter_id_lo : 8;
	uint32_t df_mcs : 3;
	uint32_t reserved : 1;
	uint32_t transmit_power : 4;
	uint32_t pad : 24;
};




/* Semaphore to synchronize modem calls. */
K_SEM_DEFINE(operation_sem, 0, 1);

K_SEM_DEFINE(deinit_sem, 0, 1);
static size_t last_rx_len;

/* Callback after init operation. */
static void on_init(const struct nrf_modem_dect_phy_init_event *evt)
{
	if (evt->err) {
		LOG_ERR("Init failed, err %d", evt->err);
		exit = true;
		return;
	}

	k_sem_give(&operation_sem);
}

/* Callback after deinit operation. */
static void on_deinit(const struct nrf_modem_dect_phy_deinit_event *evt)
{
	if (evt->err) {
		LOG_ERR("Deinit failed, err %d", evt->err);
		return;
	}

	k_sem_give(&deinit_sem);
}

static void on_activate(const struct nrf_modem_dect_phy_activate_event *evt)
{
	if (evt->err) {
		LOG_ERR("Activate failed, err %d", evt->err);
		exit = true;
		return;
	}

	k_sem_give(&operation_sem);
}

static void on_deactivate(const struct nrf_modem_dect_phy_deactivate_event *evt)
{

	if (evt->err) {
		LOG_ERR("Deactivate failed, err %d", evt->err);
		return;
	}

	k_sem_give(&deinit_sem);
}

static void on_configure(const struct nrf_modem_dect_phy_configure_event *evt)
{
	if (evt->err) {
		LOG_ERR("Configure failed, err %d", evt->err);
		return;
	}

	k_sem_give(&operation_sem);
}

/* Callback after link configuration operation. */
static void on_link_config(const struct nrf_modem_dect_phy_link_config_event *evt)
{
	LOG_DBG("link_config cb time %"PRIu64" status %d", modem_time, evt->err);
}

static void on_radio_config(const struct nrf_modem_dect_phy_radio_config_event *evt)
{
	if (evt->err) {
		LOG_ERR("Radio config failed, err %d", evt->err);
		return;
	}

	k_sem_give(&operation_sem);
}

/* Callback after capability get operation. */
static void on_capability_get(const struct nrf_modem_dect_phy_capability_get_event *evt)
{
	LOG_DBG("capability_get cb time %"PRIu64" status %d", modem_time, evt->err);
}

static void on_bands_get(const struct nrf_modem_dect_phy_band_get_event *evt)
{
	LOG_DBG("bands_get cb status %d", evt->err);
}

static void on_latency_info_get(const struct nrf_modem_dect_phy_latency_info_event *evt)
{
	LOG_DBG("latency_info_get cb status %d", evt->err);
}

/* Callback after time query operation. */
static void on_time_get(const struct nrf_modem_dect_phy_time_get_event *evt)
{
	LOG_DBG("time_get cb time %"PRIu64" status %d", modem_time, evt->err);
}

static void on_cancel(const struct nrf_modem_dect_phy_cancel_event *evt)
{
	LOG_DBG("on_cancel cb status %d", evt->err);
	k_sem_give(&operation_sem);
}

/* Operation complete notification. */
static void on_op_complete(const struct nrf_modem_dect_phy_op_complete_event *evt)
{
	LOG_DBG("op_complete cb time %"PRIu64" status %d", modem_time, evt->err);
	k_sem_give(&operation_sem);
}

/* Physical Control Channel reception notification. */
static void on_pcc(const struct nrf_modem_dect_phy_pcc_event *evt)
{
	LOG_INF("Received header from device ID %d",
		evt->hdr.hdr_type_1.transmitter_id_hi << 8 | evt->hdr.hdr_type_1.transmitter_id_lo);
}

/* Physical Control Channel CRC error notification. */
static void on_pcc_crc_err(const struct nrf_modem_dect_phy_pcc_crc_failure_event *evt)
{
	LOG_DBG("pcc_crc_err cb time %"PRIu64"", modem_time);
}

/* Physical Data Channel reception notification. */
static void on_pdc(const struct nrf_modem_dect_phy_pdc_event *evt)
{
    const uint8_t *p = (const uint8_t *)evt->data;
    size_t n = evt->len;

    if (p == NULL || n == 0) {
        return;
    }

    /* Clamp and store RX payload for ping/fping threads */
    size_t copy_len = n;
    if (copy_len > sizeof(last_rx_buf)) {
        copy_len = sizeof(last_rx_buf);
    }
    memcpy(last_rx_buf, p, copy_len);
    last_rx_len = copy_len;

    LOG_INF("PDC frame received, len=%u", (unsigned int)n);

    /* =========================================================
     * EXISTING PING / MAC TEXT HANDLING (same idea, safer prints)
     * ========================================================= */
    bool is_ping = false;
    bool is_pong = false;

    if (n >= 4) {
        if (memcmp(p, "PING", 4) == 0) {
            is_ping = true;
        } else if (memcmp(p, "PONG", 4) == 0) {
            is_pong = true;
        }
    }

    /* Print payload safely (not null-terminated) */
    size_t print_len = n;
    if (print_len > 64) {     /* limit log spam */
        print_len = 64;
    }

    if (is_ping || is_pong) {
        LOG_INF("[PING] payload (first %u bytes): %.*s",
                (unsigned int)print_len, (int)print_len, (const char *)p);

        if (is_pong) {
            /* Only for PONG */
            mac_rtt_on_pong_rx_modem_time(modem_time);
        }
    } else {
        LOG_INF("[MAC ] payload (first %u bytes): %.*s",
                (unsigned int)print_len, (int)print_len, (const char *)p);
    }

    /* wake waiting thread (ping/fping uses operation_sem) */
    k_sem_give(&operation_sem);
}

/* Physical Data Channel CRC error notification. */
static void on_pdc_crc_err(const struct nrf_modem_dect_phy_pdc_crc_failure_event *evt)
{
	LOG_DBG("pdc_crc_err cb time %"PRIu64"", modem_time);
}

/* RSSI measurement result notification. */
static void on_rssi(const struct nrf_modem_dect_phy_rssi_event *evt)
{
	LOG_DBG("rssi cb time %"PRIu64" carrier %d", modem_time, evt->carrier);
}

static void on_stf_cover_seq_control(const struct nrf_modem_dect_phy_stf_control_event *evt)
{
	LOG_WRN("Unexpectedly in %s\n", (__func__));
}

static void dect_phy_event_handler(const struct nrf_modem_dect_phy_event *evt)
{
	modem_time = evt->time;

	switch (evt->id) {
	case NRF_MODEM_DECT_PHY_EVT_INIT:
		on_init(&evt->init);
		break;
	case NRF_MODEM_DECT_PHY_EVT_DEINIT:
		on_deinit(&evt->deinit);
		break;
	case NRF_MODEM_DECT_PHY_EVT_ACTIVATE:
		on_activate(&evt->activate);
		break;
	case NRF_MODEM_DECT_PHY_EVT_DEACTIVATE:
		on_deactivate(&evt->deactivate);
		break;
	case NRF_MODEM_DECT_PHY_EVT_CONFIGURE:
		on_configure(&evt->configure);
		break;
	case NRF_MODEM_DECT_PHY_EVT_RADIO_CONFIG:
		on_radio_config(&evt->radio_config);
		break;
	case NRF_MODEM_DECT_PHY_EVT_COMPLETED:
		on_op_complete(&evt->op_complete);
		break;
	case NRF_MODEM_DECT_PHY_EVT_CANCELED:
		on_cancel(&evt->cancel);
		break;
	case NRF_MODEM_DECT_PHY_EVT_RSSI:
		on_rssi(&evt->rssi);
		break;
	case NRF_MODEM_DECT_PHY_EVT_PCC:
		mac_rtt_on_tx_done_modem_time(modem_time);
		on_pcc(&evt->pcc);
		break;
	case NRF_MODEM_DECT_PHY_EVT_PCC_ERROR:
		on_pcc_crc_err(&evt->pcc_crc_err);
		break;
	case NRF_MODEM_DECT_PHY_EVT_PDC:
		on_pdc(&evt->pdc);
		break;
	case NRF_MODEM_DECT_PHY_EVT_PDC_ERROR:
		on_pdc_crc_err(&evt->pdc_crc_err);
		break;
	case NRF_MODEM_DECT_PHY_EVT_TIME:
		on_time_get(&evt->time_get);
		break;
	case NRF_MODEM_DECT_PHY_EVT_CAPABILITY:
		on_capability_get(&evt->capability_get);
		break;
	case NRF_MODEM_DECT_PHY_EVT_BANDS:
		on_bands_get(&evt->band_get);
		break;
	case NRF_MODEM_DECT_PHY_EVT_LATENCY:
		on_latency_info_get(&evt->latency_get);
		break;
	case NRF_MODEM_DECT_PHY_EVT_LINK_CONFIG:
		on_link_config(&evt->link_config);
		break;
	case NRF_MODEM_DECT_PHY_EVT_STF_CONFIG:
		on_stf_cover_seq_control(&evt->stf_cover_seq_control);
		break;
	}
}

/* Dect PHY config parameters. */
static struct nrf_modem_dect_phy_config_params dect_phy_config_params = {
    .band_group_index       = ((CONFIG_CARRIER >= 525 && CONFIG_CARRIER <= 551)) ? 1 : 0,
    .harq_rx_process_count  = 4,
    .harq_rx_expiry_time_us = 5000000,
};

/* Send operation. */
int transmit(uint32_t handle, void *data, size_t data_len)
{
    int err;

    struct phy_ctrl_field_common header = {
        .header_format      = 0x0,
        .packet_length_type = 0x0,
        .packet_length      = 0x01,
        .short_network_id   = (hs_network_id & 0xff),   /* or CONFIG_NETWORK_ID if you keep it fixed */
        .transmitter_id_hi  = (hs_device_id >> 8),
        .transmitter_id_lo  = (hs_device_id & 0xff),
        .transmit_power     = hs_tx_power,              /* <-- runtime */
        .reserved           = 0,
        .df_mcs             = hs_mcs,                   /* <-- runtime */
    };

    struct nrf_modem_dect_phy_tx_params tx_op_params = {
        .start_time             = 0,
        .handle                 = handle,
        .network_id             = hs_network_id,        /* or CONFIG_NETWORK_ID */
        .phy_type               = 0,
        .lbt_rssi_threshold_max = 0,
        .carrier                = hs_carrier,           /* <-- runtime */
        .lbt_period             = NRF_MODEM_DECT_LBT_PERIOD_MAX,
        .phy_header             = (union nrf_modem_dect_phy_hdr *)&header,
        .data                   = data,
        .data_size              = data_len,
    };

    err = nrf_modem_dect_phy_tx(&tx_op_params);
    return err;
}

/* Receive operation. */
 int receive(uint32_t handle)
{
    int err;

    struct nrf_modem_dect_phy_rx_params rx_op_params = {
        .start_time   = 0,
        .handle       = handle,
        .network_id   = hs_network_id,   /* or CONFIG_NETWORK_ID */
        .mode         = NRF_MODEM_DECT_PHY_RX_MODE_CONTINUOUS,
        .rssi_interval = NRF_MODEM_DECT_PHY_RSSI_INTERVAL_OFF,
        .link_id      = NRF_MODEM_DECT_PHY_LINK_UNSPECIFIED,
        .rssi_level   = -60,
        .carrier      = hs_carrier,      /* <-- runtime */
        .duration     = CONFIG_RX_PERIOD_S * MSEC_PER_SEC *
                        NRF_MODEM_DECT_MODEM_TIME_TICK_RATE_KHZ,
        .filter.short_network_id        = hs_network_id & 0xff,
        .filter.is_short_network_id_used = 1,
        .filter.receiver_identity       = 0,
    };

    err = nrf_modem_dect_phy_rx(&rx_op_params);
    return err;
}

/* Session Start For the initialization of the Board    t */
int dect_session_open(void)
{
    int err;

    err = nrf_modem_lib_init();
    if (err) {
        LOG_ERR("modem init failed, err %d", err);
        return err;
    }

    err = nrf_modem_dect_phy_event_handler_set(dect_phy_event_handler);
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_event_handler_set failed, err %d", err);
        return err;
    }

    err = nrf_modem_dect_phy_init();
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_init failed, err %d", err);
        return err;
    }

    k_sem_take(&operation_sem, K_FOREVER);
    if (exit) {
        return -EIO;
    }

    err = nrf_modem_dect_phy_configure(&dect_phy_config_params);
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_configure failed, err %d", err);
        return err;
    }

    k_sem_take(&operation_sem, K_FOREVER);
    if (exit) {
        return -EIO;
    }

    err = nrf_modem_dect_phy_activate(NRF_MODEM_DECT_PHY_RADIO_MODE_LOW_LATENCY);
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_activate failed, err %d", err);
        return err;
    }

    k_sem_take(&operation_sem, K_FOREVER);
    if (exit) {
        return -EIO;
    }

    hwinfo_get_device_id((void *)&device_id, sizeof(device_id));
    LOG_INF("DECT NR+ PHY initialized, device ID: %d", device_id);

    err = nrf_modem_dect_phy_capability_get();
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_capability_get failed, err %d", err);
        /* not fatal */
    }

    return 0;
}

int dect_session_close(void)
{
    int err;

    LOG_INF("Shutting down DECT session");

    err = nrf_modem_dect_phy_deactivate();
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_deactivate failed, err %d", err);
        return err;
    }

    k_sem_take(&deinit_sem, K_FOREVER);

    err = nrf_modem_dect_phy_deinit();
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_deinit() failed, err %d", err);
        return err;
    }

    k_sem_take(&deinit_sem, K_FOREVER);

    err = nrf_modem_lib_shutdown();
    if (err) {
        LOG_ERR("nrf_modem_lib_shutdown() failed, err %d", err);
        return err;
    }

    LOG_INF("DECT session closed");
    return 0;
}
/*End of the Session functions  */
 
/* ============================================================
 *               CONFIGURATION GETTERS
 * ============================================================ */
int hs_core_get_config(struct hs_config *cfg)
{
    if (!cfg) return -EINVAL;

    cfg->carrier       = hs_carrier;
    cfg->band_group_idx = hs_band_group;
    cfg->mcs           = hs_mcs;
    cfg->tx_power      = hs_tx_power;
    cfg->network_id    = hs_network_id;
    cfg->device_id     = hs_device_id;

    return 0;
}

/* ============================================================
 *               CONFIGURATION VALIDATION HELPERS
 * ============================================================ */
static bool carrier_valid(int c)
{
    return (c >= HS_CARRIER_MIN &&
            c <= HS_CARRIER_MAX &&
            (c % 2) == 1);              /* must be odd */
}

static bool mcs_valid(int m)
{
    return (m >= HS_MCS_MIN && m <= HS_MCS_MAX);
}

static bool txp_valid(int p)
{
    return (p >= HS_TXPWR_MIN_DBM && p <= HS_TXPWR_MAX_DBM);
}

/* ============================================================
 *        APPLY RADIO CONFIG (used by all setters)
 * ============================================================ */
static int apply_radio_config(void)
{
    int err;

    /* Only band group is configured via dect_phy_config_params in this SDK */
    dect_phy_config_params.band_group_index = hs_band_group;

    err = nrf_modem_dect_phy_configure(&dect_phy_config_params);
    if (err) {
        LOG_ERR("nrf_modem_dect_phy_configure failed, err %d", err);
        return err;
    }

    return 0;
}


/* ============================================================
 *                 SETTERS WITH VALIDATION
 * ============================================================ */

int static hs_core_set_carrier(int carrier)
{
    if (!carrier_valid(carrier)) {
        LOG_ERR("Invalid carrier %d (must be odd, 1657–1677)", carrier);
        return -EINVAL;
    }

    hs_carrier = carrier;
    return 0;
}

int static hs_core_set_mcs(int mcs)
{
    if (!mcs_valid(mcs)) {
        LOG_ERR("Invalid MCS %d", mcs);
        return -EINVAL;
    }

    hs_mcs = mcs;
    return 0;
}

int static hs_core_set_tx_power(int pwr)
{
    if (!txp_valid(pwr)) {
        LOG_ERR("Invalid TX power %d", pwr);
        return -EINVAL;
    }

    hs_tx_power = pwr;
    return 0;
}

int hs_core_apply_radio(uint16_t carrier, uint8_t mcs, int8_t tx_power)
{
    int err;

   
    err = hs_core_set_carrier(carrier);
    if (err) {
        LOG_ERR("hs_core_apply_radio: carrier %u rejected (err=%d)",
                carrier, err);
        return err;
    }

    
    err = hs_core_set_mcs(mcs);
    if (err) {
        LOG_ERR("hs_core_apply_radio: MCS %u rejected (err=%d)",
                mcs, err);
        return err;
    }

   
    err = hs_core_set_tx_power(tx_power);
    if (err) {
        LOG_ERR("hs_core_apply_radio: TX power %d rejected (err=%d)",
                tx_power, err);
        return err;
    }

    
    return 0;
}

int core_get_last_rx(uint8_t *dst, size_t max_len, size_t *out_len)
{
    if (!dst || !out_len) return -EINVAL;
    size_t n = last_rx_len;
    if (n > max_len) n = max_len;
    memcpy(dst, last_rx_buf, n);
    *out_len = n;
    return 0;
}
