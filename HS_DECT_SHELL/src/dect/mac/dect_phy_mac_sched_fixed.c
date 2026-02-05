#include <zephyr/kernel.h>
#include <nrf_modem_dect_phy.h>
#include "dect_common_settings.h"
#include "dect_phy_mac_sched_fixed.h"
#include "dect_app_time.h"  /* provides dect_app_modem_time_now() */

#ifndef DECT_SUBSLOT_BB_TICKS
/* subslot duration in modem ticks . */
#define DECT_SUBSLOT_BB_TICKS (28800ULL)
#endif

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
	if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
		return -EINVAL;
	}
	if (s->mac_sched.pt_id == 0) {
		/* FT does not transmit UL. */
		return -EINVAL;
	}
	if (dect_phy_mac_sched_fixed_validate_settings() != 0) {
		return -EINVAL;
	}

	uint16_t st = 0, en = 0;
	if (dect_phy_mac_sched_fixed_slot_get(s->mac_sched.pt_id, &st, &en) != 0) {
		return -EINVAL;
	}

	uint64_t now = dect_app_modem_time_now();
	uint64_t sf_ticks = superframe_bb_ticks_get(s->mac_sched.superframe_len);

	/* Align "now" to superframe boundary (simple modulo alignment). */
	uint64_t sf_start = now - (now % sf_ticks);

	uint64_t slot_start = sf_start + ((uint64_t)st * DECT_SUBSLOT_BB_TICKS);

	/* If already past slot_start, schedule in next superframe. */
	if (slot_start <= now) {
		slot_start += sf_ticks;
	}

	*start_time_bb = slot_start;
	return 0;
}
