#pragma once

#include <stdint.h>
#include <stdbool.h>

bool dect_phy_mac_sched_fixed_enabled(void);
int dect_phy_mac_sched_fixed_validate_settings(void);
int dect_phy_mac_sched_fixed_slot_get(uint8_t pt_id, uint16_t *start, uint16_t *end);

/* Returns the next permitted UL start time in modem baseband ticks. */
int dect_phy_mac_sched_fixed_next_ul_start_time_get(uint64_t *start_time_bb);
