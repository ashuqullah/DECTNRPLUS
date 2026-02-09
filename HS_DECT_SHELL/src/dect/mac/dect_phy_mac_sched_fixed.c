#include <zephyr/kernel.h>
#include <nrf_modem_dect_phy.h>
#include "dect_common_settings.h"
#include "dect_phy_mac_sched_fixed.h"
#include "dect_app_time.h"  /* provides dect_app_modem_time_now() */

#ifndef DECT_SUBSLOT_BB_TICKS
/* subslot duration in modem ticks . */
#define DECT_SUBSLOT_BB_TICKS (28800ULL)
#endif
/* DECT ETSI timing constants*/
#define DECT_ETSI_FRAME_US            10000U  /* 10 ms */
#define DECT_ETSI_SLOT_US               416U  /* ~416 us */
#define DECT_ETSI_SLOTS_PER_FRAME        24U  /* 24*416 */


bool dect_phy_mac_sched_fixed_enabled(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();
	return (s->mac_sched.mode == DECT_MAC_SCHED_FIXED);
}

int dect_phy_mac_sched_fixed_validate_settings(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();

	if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
		return 0;
	}
	if (s->mac_sched.superframe_len == 0) {
		return -EINVAL;
	}
	if (s->mac_sched.max_pts < 1 || s->mac_sched.max_pts > DECT_MAX_PTS) {
		return -EINVAL;
	}
	if (s->mac_sched.pt_id != 0 && s->mac_sched.pt_id > s->mac_sched.max_pts) {
		return -EINVAL;
	}
	for (int i = 0; i < s->mac_sched.max_pts; i++) {
		uint16_t st = s->mac_sched.pt_slots[i].start_subslot;
		uint16_t en = s->mac_sched.pt_slots[i].end_subslot;
		if (st > en || en >= s->mac_sched.superframe_len) {
			return -EINVAL;
		}
	}
	return 0;
}

int dect_phy_mac_sched_fixed_slot_get(uint8_t pt_id, uint16_t *start, uint16_t *end)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();
	if (!start || !end) {
		return -EINVAL;
	}
	if (pt_id == 0 || pt_id > s->mac_sched.max_pts) {
		return -EINVAL;
	}
	int idx = (int)pt_id - 1;
	*start = s->mac_sched.pt_slots[idx].start_subslot;
	*end = s->mac_sched.pt_slots[idx].end_subslot;
	return 0;
}

static uint64_t superframe_bb_ticks_get(uint16_t sf_len)
{
	return (uint64_t)sf_len * DECT_SUBSLOT_BB_TICKS;
}

int dect_phy_mac_sched_fixed_next_ul_start_time_get(uint64_t *start_time_bb)
{
	if (!start_time_bb) {
		return -EINVAL;
	}

	struct dect_phy_settings *s = dect_common_settings_ref_get();

	/* Only valid in fixed scheduling mode and on PT side */
	if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
		return -EINVAL;
	}
	if (s->mac_sched.pt_id == 0) {
		/* FT does not use PT UL scheduling */
		return -EINVAL;
	}

	/* Convert ETSI timing to modem BB ticks */
	const uint64_t frame_ticks = dect_app_time_us_to_mdm_ticks(DECT_ETSI_FRAME_US);
	const uint64_t slot_ticks  = dect_app_time_us_to_mdm_ticks(DECT_ETSI_SLOT_US);

	if (frame_ticks == 0 || slot_ticks == 0) {
		return -EINVAL;
	}

	/* Read PT slot assignment (interpreted as SLOT INDICES inside a 10ms frame) */
	uint16_t slot_start = 0, slot_end = 0;
	int ret = dect_phy_mac_sched_fixed_slot_get(s->mac_sched.pt_id, &slot_start, &slot_end);
	if (ret) {
		return ret;
	}

	/* Validate slot range fits inside one ETSI frame */
	if (slot_start > slot_end ||
	    slot_end >= DECT_ETSI_SLOTS_PER_FRAME) {
		return -EINVAL;
	}

	/* Choose which slot to use.
	 * Minimal deterministic behavior: always transmit at slot_start.
	 * (Later you can rotate between slot_start..slot_end.)
	 */
	const uint16_t chosen_slot = slot_start;

	uint64_t now = dect_app_modem_time_now();

	/* Align to 10 ms frame boundary */
	uint64_t frame_start = now - (now % frame_ticks);

	/* Compute the next TX time at the chosen slot boundary */
	uint64_t tx_time = frame_start + ((uint64_t)chosen_slot * slot_ticks);

	/* If already passed in this frame, schedule in next frame */
	if (tx_time <= now) {
		tx_time += frame_ticks;
	}

	*start_time_bb = tx_time;
	return 0;
}
