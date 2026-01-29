#ifndef HS_CORE_H_
#define HS_CORE_H_

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
/* === Limits from DECT modem capabilities === */

/* From "Min carrier: 1657 / Max carrier: 1677", band group 0, odd only */
#define HS_CARRIER_MIN     1657
#define HS_CARRIER_MAX     1677

/* From "mcs_max[0]: 4" */
#define HS_MCS_MIN         0
#define HS_MCS_MAX         4

/* From "Max power: 19dBm" (band 1, power class 3) */
#define HS_TXPWR_MIN_DBM  -30   /* chosen low bound – safe margin */
#define HS_TXPWR_MAX_DBM   19   /* from modem band info */

/* Public config struct */
struct hs_config {
    uint16_t    device_id;
    uint32_t    network_id;
    uint16_t    carrier;
    uint8_t     band_group_idx;
    uint8_t     mcs;
    int8_t      tx_power;
};

int hs_core_get_config(struct hs_config *out);
int hs_core_apply_radio(uint16_t carrier, uint8_t mcs, int8_t tx_power);
int hs_core_set_network_id(uint32_t netid);
int dect_session_open(void);
int dect_session_close(void);

int transmit(uint32_t handle, void *data, size_t data_len);
int receive(uint32_t handle);

void mac_rtt_mark_tx(void);
void mac_rtt_set_enabled(bool enable);

#endif /* HS_CORE_H_ */
