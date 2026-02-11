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

    if (s == NULL) {
        return -EINVAL;
    }

    if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
        return 0; /* Not fixed scheduling -> always valid here */
    }

    /* Validate fixed scheduling against a single 10ms frame of 24 slots (0..23).
     * superframe_len may be 0 during bootstrap and must not invalidate FIXED mode.
     */
    const uint16_t frame_len_slots = 24;

    /* Role must be set in FIXED mode */
    if (s->mac_sched.role != DECT_MAC_ROLE_FT && s->mac_sched.role != DECT_MAC_ROLE_PT) {
        return -EINVAL;
    }

    /* ---------------- PT validation ---------------- */
    if (s->mac_sched.role == DECT_MAC_ROLE_PT) {
        /* PT must have a valid PT ID (1..DECT_MAX_PTS) */
        if (s->mac_sched.pt_id < 1 || s->mac_sched.pt_id > DECT_MAX_PTS) {
            return -EINVAL;
        }

        /* PT does not need max_pts or slot-table validation here.
         * Slot ownership can be derived/received from FT later.
         */
        return 0;
    }

    /* ---------------- FT validation ---------------- */
    /* FT must have max_pts within [1..DECT_MAX_PTS] */
    if (s->mac_sched.max_pts < 1 || s->mac_sched.max_pts > DECT_MAX_PTS) {
        return -EINVAL;
    }

    /* Validate each configured PT slot range:
     * - start <= end
     * - end within 0..23
     * - no overlap between PT ranges
     */
    for (int i = 0; i < s->mac_sched.max_pts; i++) {
        uint16_t st_i = s->mac_sched.pt_slots[i].start_subslot;
        uint16_t en_i = s->mac_sched.pt_slots[i].end_subslot;

        if (st_i > en_i) {
            return -EINVAL;
        }
        if (en_i >= frame_len_slots) {
            return -EINVAL;
        }

        /* overlap check with later entries */
        for (int j = i + 1; j < s->mac_sched.max_pts; j++) {
            uint16_t st_j = s->mac_sched.pt_slots[j].start_subslot;
            uint16_t en_j = s->mac_sched.pt_slots[j].end_subslot;

            if (st_j > en_j) {
                return -EINVAL;
            }
            if (en_j >= frame_len_slots) {
                return -EINVAL;
            }

            bool overlap = (st_i <= en_j) && (st_j <= en_i);
            if (overlap) {
                return -EINVAL;
            }
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
