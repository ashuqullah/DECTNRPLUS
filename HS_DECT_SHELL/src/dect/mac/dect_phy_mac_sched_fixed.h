#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "dect_phy_mac_pdu.h"
/* Handle range reserved for FT "fixed join" RX windows scheduled from beaconing. */
#define DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_START  1600
#define DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_END    1699

/* Default JOIN/ASSOC RX window length in frames. */
#define DECT_PHY_MAC_FIXED_JOIN_RX_FRAMES_DEFAULT       20

bool dect_phy_mac_sched_fixed_enabled(void);
bool dect_phy_mac_sched_reallocation_enabled(void);
bool dect_phy_mac_sched_random_enabled(void);

/* Validate fixed scheduling settings in dect_common_settings.
 * Returns 0 if valid (or not in fixed mode), negative errno otherwise.
 */
int dect_phy_mac_sched_fixed_validate_settings(void);

/* Get PT allocation as SUBSLOT indices inside a 10 ms DECT frame.
 * pt_id is 1-based.
 */
int dect_phy_mac_sched_fixed_slot_get(uint8_t pt_id, uint16_t *start_subslot, uint16_t *end_subslot);

/* Returns the next permitted UL start time in modem ticks (PT side). */
int dect_phy_mac_sched_fixed_next_ul_start_time_get(uint64_t *start_time_mdm_ticks);

/* Build FIXED scheduling payload for beacon SDU (FT side).
 * Encoding: [version][mode][max_pts][slots_per_frame][start_slot,end_slot]*max_pts
 * Returns payload length on success, negative errno on error.
 */
int dect_phy_mac_sched_fixed_build_beacon_payload(uint8_t *buf, size_t buf_size);

/* FT helper: schedule periodic RX windows for JOIN/ASSOC in the PT1 allocation region. */
int dect_phy_mac_ft_fixed_join_rx_schedule_start(uint64_t beacon_start_time,
                                                 uint16_t channel,
                                                 uint32_t beacon_interval_mdm_ticks);

/* Convert PT allocation (subslots) to SLOT indices (inclusive) inside a frame. */
int dect_phy_mac_sched_fixed_pt_slot_range_get(uint8_t pt_idx,
                                               uint8_t max_pts,
                                               uint8_t *start_slot,
                                               uint8_t *end_slot);


void dect_phy_mac_fixed_sched_resource_ie_handle(
	const dect_phy_mac_common_header_t *common_header,
	const dect_phy_mac_fixed_sched_resource_ie_t *ie);