#include <zephyr/kernel.h>
#include <nrf_modem_dect_phy.h>
#include "dect_common_settings.h"
#include "dect_phy_mac_sched_fixed.h"
#include "dect_app_time.h"  /* provides dect_app_modem_time_now() */
#include "dect_common_utils.h"
#include "dect_common.h"

#include "dect_phy_api_scheduler.h"   /* scheduler list item types + alloc/add/dealloc */
#include <errno.h>
#include "desh_print.h"               /* desh_print / desh_warn (or whatever your project uses) */
#include "dect_phy_mac.h"             /* DECT_PRIORITY2_RX and other MAC priorities/handles */

/*  shared functions*/

bool hsa_dect_slots_overlap(uint16_t a_start, uint16_t a_end,
                            uint16_t b_start, uint16_t b_end)
{
    return !(a_end < b_start || b_end < a_start);
}

void hsa_dect_assign_default_pt_slots(struct dect_phy_settings *s)
{
    int n = s->mac_sched.max_pts;

    if (n < 1) {
        n = DECT_DEF_PTS;
    }
    if (n > DECT_MAX_PTS) {
        n = DECT_MAX_PTS;
    }

    const uint16_t total  = DECT_RADIO_FRAME_SUBSLOT_COUNT; /* 48 */
    const uint16_t guard  = DECT_MAC_UL_GUARD_SUBSLOTS;     /* 8  */
    const uint16_t usable = (guard < total) ? (total - guard) : 0;

    if (usable == 0) {
        for (int i = 0; i < DECT_MAX_PTS; i++) {
            s->mac_sched.pt_slots[i].start_subslot = 0;
            s->mac_sched.pt_slots[i].end_subslot   = 0;
        }
        return;
    }

    const uint16_t base = usable / n;
    const uint16_t rem  = usable % n;

    uint16_t start = guard;

    for (int i = 0; i < n; i++) {
        uint16_t count = base + ((i < rem) ? 1 : 0);
        uint16_t end   = (count > 0) ? (start + count - 1) : start;

        s->mac_sched.pt_slots[i].start_subslot = start;
        s->mac_sched.pt_slots[i].end_subslot   = end;

        start = end + 1;
    }

    for (int i = n; i < DECT_MAX_PTS; i++) {
        s->mac_sched.pt_slots[i].start_subslot = 0;
        s->mac_sched.pt_slots[i].end_subslot   = 0;
    }
}


/* Auto-allocation: reserve slot 0 for beacon/control, split remaining slots among PTs. */
static void fixed_auto_pt_slot_range_compute(uint8_t pt_idx0, uint8_t max_pts,
                                             uint8_t *start_slot, uint8_t *end_slot)
{
    /* Guard is defined in SUBSLOTS; convert to whole slots for slot-domain splitting */
    const uint8_t ss_per_slot = (uint8_t)DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT;
    const uint8_t guard_slots = (uint8_t)((DECT_MAC_UL_GUARD_SUBSLOTS + (ss_per_slot - 1U)) / ss_per_slot);

    const uint8_t first_slot  = guard_slots;
    const uint8_t total_slots = (uint8_t)DECT_RADIO_FRAME_SLOT_COUNT - first_slot;
    const uint8_t base = total_slots / max_pts;
    const uint8_t rem  = total_slots % max_pts;

    uint8_t start = first_slot;
    for (uint8_t i = 0; i < pt_idx0; i++) {
        start += (uint8_t)(base + ((i < rem) ? 1 : 0));
    }

    uint8_t len = (uint8_t)(base + ((pt_idx0 < rem) ? 1 : 0));

    *start_slot = start;
    *end_slot   = (uint8_t)(start + len - 1);
}

static bool fixed_table_is_configured(const struct dect_phy_settings *s)
{
    /* Treat table as "configured" if any PT has a non-zero range. */
    for (int i = 0; i < s->mac_sched.max_pts; i++) {
        if (s->mac_sched.pt_slots[i].start_subslot != 0 ||
            s->mac_sched.pt_slots[i].end_subslot   != 0) {
            return true;
        }
    }
    return false;
}

bool dect_phy_mac_sched_fixed_enabled(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();
	return (s->mac_sched.mode == DECT_MAC_SCHED_FIXED);
}
bool dect_phy_mac_sched_reallocation_enabled(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();
	return (s->mac_sched.mode == DECT_MAC_SCHED_RALLOCATE);
}
bool dect_phy_mac_sched_ra_enabled(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();
	return (s->mac_sched.mode == DECT_MAC_SCHED_RANDOM);
}
int dect_phy_mac_sched_fixed_validate_settings(void)
{
    struct dect_phy_settings *s = dect_common_settings_ref_get();

    if (s == NULL) {
        return -EINVAL;
    }

    /* Not fixed scheduling -> nothing to validate here. */
    if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
        return 0;
    }

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
        return 0;
    }

    /* ---------------- FT validation ---------------- */
    /* FT must have max_pts within [1..DECT_MAX_PTS] */
    if (s->mac_sched.max_pts < 1 || s->mac_sched.max_pts > DECT_MAX_PTS) {
        return -EINVAL;
    }
	const bool table_cfg = fixed_table_is_configured(s);


		/* If table not configured -> accept (auto allocation will be used). */
		if (!table_cfg) {
			return 0;
		}
    /* Validate each configured PT allocation as SUBSLOT indices inside one 10 ms frame:
     * - start <= end
     * - end within frame subslot range
     * - no overlap between PT ranges
     */
    const uint16_t frame_len_subslots = DECT_RADIO_FRAME_SUBSLOT_COUNT;

    for (int i = 0; i < s->mac_sched.max_pts; i++) {
        uint16_t st_i = s->mac_sched.pt_slots[i].start_subslot;
        uint16_t en_i = s->mac_sched.pt_slots[i].end_subslot;

        /* Unused entry is allowed */
        if (st_i == 0U && en_i == 0U) {
            continue;
        }
        /* Enforce guard region at frame start */
        if (st_i < DECT_MAC_UL_GUARD_SUBSLOTS) {
            return -EINVAL;
        }

        if (st_i > en_i) {
            return -EINVAL;
        }
        if (en_i >= frame_len_subslots) {
            return -EINVAL;
        }

        for (int j = i + 1; j < s->mac_sched.max_pts; j++) {
            uint16_t st_j = s->mac_sched.pt_slots[j].start_subslot;
            uint16_t en_j = s->mac_sched.pt_slots[j].end_subslot;

            /* Unused entry is allowed */
            if (st_j == 0U && en_j == 0U) {
                continue;
            }
            /* Enforce guard region at frame start */
            if (st_j < DECT_MAC_UL_GUARD_SUBSLOTS) {
                return -EINVAL;
            }

            if (st_j > en_j) {
                return -EINVAL;
            }
            if (en_j >= frame_len_subslots) {
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

int dect_phy_mac_sched_fixed_pt_slot_range_get(uint8_t pt_id,
                                               uint8_t max_pts,
                                               uint8_t *start_slot,
                                               uint8_t *end_slot)
{
    ARG_UNUSED(max_pts);

    if (!start_slot || !end_slot) {
        return -EINVAL;
    }

    /* dect_phy_mac_sched_fixed_slot_get() returns start/end in the configured unit.
     * Today your settings may store either:
     *  - slots  (0..DECT_RADIO_FRAME_SLOT_COUNT-1), OR
     *  - subslots (0..DECT_RADIO_FRAME_SUBSLOT_COUNT-1)
     *
     * Option B: detect which one and convert consistently to slots.
     */
    uint16_t start_cfg = 0;
    uint16_t end_cfg   = 0;

    int ret = dect_phy_mac_sched_fixed_slot_get(pt_id, &start_cfg, &end_cfg);
    if (ret) {
        return ret;
    }

    if (end_cfg < start_cfg) {
        return -EINVAL;
    }

    /* Settings are ALWAYS in SUBSLOTS now. Convert to slot indices for slot-based users. */
    if (end_cfg >= DECT_RADIO_FRAME_SUBSLOT_COUNT || start_cfg >= DECT_RADIO_FRAME_SUBSLOT_COUNT) {
        return -EINVAL;
    }

    *start_slot = (uint8_t)(start_cfg / DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);
    *end_slot   = (uint8_t)(end_cfg   / DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);

    if (*end_slot >= DECT_RADIO_FRAME_SLOT_COUNT) {
        return -EINVAL;
    }

    return 0;
}



int dect_phy_mac_sched_fixed_slot_get(uint8_t pt_id, uint16_t *start_subslot, uint16_t *end_subslot)
{
    struct dect_phy_settings *s = dect_common_settings_ref_get();
    if (!s || !start_subslot || !end_subslot) {
        return -EINVAL;
    }

    if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
        return -EINVAL;
    }

    if (pt_id < 1 || pt_id > s->mac_sched.max_pts) {
        return -EINVAL;
    }

    const uint8_t idx0 = (uint8_t)(pt_id - 1);

    /* If table configured, use it. */
    if (fixed_table_is_configured(s)) {
        *start_subslot = s->mac_sched.pt_slots[idx0].start_subslot;
        *end_subslot   = s->mac_sched.pt_slots[idx0].end_subslot;
        return 0;
    }

    /* Auto allocation: compute slot range and convert to subslots. */
    uint8_t st_slot, en_slot;
    fixed_auto_pt_slot_range_compute(idx0, s->mac_sched.max_pts, &st_slot, &en_slot);

    *start_subslot = (uint16_t)(st_slot * DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);
    *end_subslot   = (uint16_t)(((en_slot + 1) * DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT) - 1);

    /* Final safety */
    if (*end_subslot >= DECT_RADIO_FRAME_SUBSLOT_COUNT) {
        return -EINVAL;
    }

    return 0;
}


int dect_phy_mac_sched_fixed_next_ul_start_time_get(uint64_t *start_time_mdm_ticks)
{
    if (!start_time_mdm_ticks) {
        return -EINVAL;
    }

    struct dect_phy_settings *s = dect_common_settings_ref_get();
    if (!s) {
        return -EINVAL;
    }

    /* Only valid in fixed scheduling mode and on PT side */
    if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
        return -EINVAL;
    }
    if (s->mac_sched.role != DECT_MAC_ROLE_PT || s->mac_sched.pt_id == 0) {
        return -EINVAL;
    }

    /* Use project "source of truth" (modem ticks) from dect_common.h */
    const uint64_t frame_ticks   = DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS;
    const uint64_t subslot_ticks = DECT_RADIO_SUBSLOT_DURATION_IN_MODEM_TICKS;

    if (frame_ticks == 0 || subslot_ticks == 0) {
        return -EINVAL;
    }

    /* Read PT allocation as SUBSLOT indices inside a 10 ms frame */
    uint16_t start_subslot = 0;
    uint16_t end_subslot = 0;
    int ret = dect_phy_mac_sched_fixed_slot_get(s->mac_sched.pt_id, &start_subslot, &end_subslot);
    if (ret) {
        return ret;
    }

    /* Validate allocation fits inside one frame */
    if (start_subslot > end_subslot || end_subslot >= DECT_RADIO_FRAME_SUBSLOT_COUNT) {
        return -EINVAL;
    }

    /* Deterministic choice: transmit at the first subslot of the allocation. */
    const uint16_t chosen_subslot = start_subslot;

    uint64_t now = dect_app_modem_time_now();

    /* Align to 10 ms frame boundary */
    uint64_t frame_start = now - (now % frame_ticks);

    /* Compute next TX time at chosen subslot boundary */
    uint64_t tx_time = frame_start + ((uint64_t)chosen_subslot * subslot_ticks);

    if (tx_time <= now) {
        tx_time += frame_ticks;
    }

    *start_time_mdm_ticks = tx_time;
    return 0;
}
int dect_phy_mac_sched_fixed_build_beacon_payload(uint8_t *buf, size_t buf_size)
{
    struct dect_phy_settings *s = dect_common_settings_ref_get();
    if (!s || !buf) {
        return -EINVAL;
    }

    if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED || s->mac_sched.role != DECT_MAC_ROLE_FT) {
        return -EINVAL;
    }

    uint8_t max_pts = s->mac_sched.max_pts;
    if (max_pts == 0 || max_pts > DECT_MAX_PTS) {
        return -EINVAL;
    }

    /* Layout:
     * [0] version
     * [1] mode (1=fixed)
     * [2] max_pts
     * [3] slots_per_frame
     * [4..] PT slot allocations (start_slot,end_slot per PT)
     */
    const uint8_t slots_per_frame = DECT_RADIO_FRAME_SLOT_COUNT;
    const size_t required_len = 4 + ((size_t)max_pts * 2);

    if (buf_size < required_len) {
        return -ENOMEM;
    }

    buf[0] = 1; /* version */
    buf[1] = 1; /* fixed mode */
    buf[2] = max_pts;
    buf[3] = slots_per_frame;

    /* Encode allocations from settings table (subslots -> slots). */
	for (uint8_t i = 0; i < max_pts; i++) {
		uint16_t st_sub, en_sub;
		int ret = dect_phy_mac_sched_fixed_slot_get((uint8_t)(i + 1), &st_sub, &en_sub);
		if (ret) {
			return ret;
		}

		uint8_t st_slot = (uint8_t)(st_sub / DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);
		uint8_t en_slot = (uint8_t)(en_sub / DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);

		buf[4 + (i * 2)]     = st_slot;
		buf[4 + (i * 2) + 1] = en_slot;
	}

    return (int)required_len;
}




int dect_phy_mac_ft_fixed_join_rx_schedule_start(uint64_t beacon_frame_time,
                                                 uint16_t channel,
                                                 uint32_t beacon_interval_mdm_ticks)
{
    static bool join_rx_already_scheduled;

    if (join_rx_already_scheduled) {
        return 0;
    }
    join_rx_already_scheduled = true;

    struct dect_phy_settings *s = dect_common_settings_ref_get();
    if (!s) {
        return -EINVAL;
    }

    const uint32_t frame_ticks = DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS;

    /* Start JOIN listen this many frames after beacon */
    const uint32_t join_offset_frames = 20;

    /* How many consecutive frames to listen */
    const uint32_t join_window_frames = DECT_PHY_MAC_FIXED_JOIN_RX_FRAMES_DEFAULT; /* e.g. 20 */

    if (beacon_interval_mdm_ticks < frame_ticks) {
        return -EINVAL;
    }

    /* Remove any old JOIN RX items */
    (void)dect_phy_api_scheduler_list_item_remove_dealloc_by_phy_op_handle_range(
        DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_START,
        DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_END);

    if ((DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_START + join_window_frames) >
        (DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_END + 1)) {
        return -ENOMEM;
    }

    /* Compute a base time in the future */
    uint64_t join_base_time = beacon_frame_time + ((uint64_t)join_offset_frames * frame_ticks);

    uint64_t now = dect_app_modem_time_now();
    uint64_t min_start = now + (2ULL * frame_ticks);
    while (join_base_time < min_start) {
        join_base_time += beacon_interval_mdm_ticks;
    }

    /* ---- IMPORTANT CHANGE ----
     * Don't listen whole frame (24 slots). That causes (a) RX-to-RX overlap due to scheduler margin,
     * and (b) blocks association response TX.
     *
     * Instead: listen a short JOIN window (PT1 range + guard).
     */
    uint8_t start_slot = 0;
    uint8_t end_slot = 0;

    /* Use PT1 advertised range if available; fallback to a small window after guard */
    if (dect_phy_mac_sched_fixed_pt_slot_range_get(1, s->mac_sched.max_pts, &start_slot, &end_slot) != 0) {
        const uint8_t ss_per_slot = (uint8_t)DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT;
        const uint8_t guard_slots = (uint8_t)((DECT_MAC_UL_GUARD_SUBSLOTS + (ss_per_slot - 1U)) / ss_per_slot);
        start_slot = guard_slots;
        end_slot   = (uint8_t)(guard_slots + 5U);
        if (end_slot >= DECT_RADIO_FRAME_SLOT_COUNT) {
            end_slot = (uint8_t)(DECT_RADIO_FRAME_SLOT_COUNT - 1U);
        }
    }

    /* Add a guard slot (but keep within frame) */
    if (end_slot + 1 < DECT_RADIO_FRAME_SLOT_COUNT) {
        end_slot++;
    }

    uint8_t length_slots = (uint8_t)(end_slot - start_slot + 1);

    for (uint32_t i = 0; i < join_window_frames; i++) {
        struct dect_phy_api_scheduler_list_item_config *conf;
        struct dect_phy_api_scheduler_list_item *item =
            dect_phy_api_scheduler_list_item_alloc_rx_element(&conf);

        if (!item || !conf) {
            if (item) {
                dect_phy_api_scheduler_list_item_dealloc(item);
            }
            return -ENOMEM;
        }

        conf->cb_op_completed = NULL;
        conf->channel = channel;

        conf->frame_time = join_base_time + ((uint64_t)i * frame_ticks);

        /* ---- IMPORTANT CHANGE ----
         * No repeating JOIN RX while debugging association. Avoid long-term conflicts.
         */
        conf->interval_mdm_ticks = beacon_interval_mdm_ticks ; /* effectively disable repeating */

        conf->start_slot = start_slot;
        conf->length_slots = length_slots;
        conf->length_subslots = 0;

        conf->rx.mode = NRF_MODEM_DECT_PHY_RX_MODE_CONTINUOUS;
        conf->rx.expected_rssi_level = s->rx.expected_rssi_level;
        conf->rx.duration = 0; /* use length_slots */
        conf->rx.network_id = s->common.network_id;

        conf->rx.filter.is_short_network_id_used = true;
        conf->rx.filter.short_network_id = (uint8_t)(s->common.network_id & 0xFF);
        conf->rx.filter.receiver_identity = s->common.short_rd_id;

        /* Keep JOIN RX lower priority so TX responses can be scheduled */
        item->priority = DECT_PRIORITY2_RX;

        item->phy_op_handle =
            (uint32_t)(DECT_PHY_MAC_BEACON_RX_FIXED_JOIN_HANDLE_START + i);

        if (!dect_phy_api_scheduler_list_item_add(item)) {
            dect_phy_api_scheduler_list_item_dealloc(item);
            return -EBUSY;
        }
    }

    desh_print("FIXED JOIN RX scheduled: %u frames @ +%u frames after beacon, repeat %u ticks, ch %u, slots %u..%u (len %u)",
           join_window_frames, join_offset_frames, beacon_interval_mdm_ticks,
           channel, start_slot, end_slot, length_slots);

    return 0;
}



/* Convert slot range (0..23) to subslot range (0..47):
 * slot N => UL/DL subslots are (2*N) and (2*N + 1)
 */
static inline uint8_t slot_to_start_subslot(uint8_t slot)
{
	return (uint8_t)(slot * DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);
}

static inline uint8_t slot_to_end_subslot(uint8_t slot)
{
	return (uint8_t)((slot * DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT) +
			 (DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT - 1U));
}

void dect_phy_mac_fixed_sched_resource_ie_handle(
	const dect_phy_mac_common_header_t *common_header,
	const dect_phy_mac_fixed_sched_resource_ie_t *ie)
{
	ARG_UNUSED(common_header);

	struct dect_phy_settings *s = dect_common_settings_ref_get();
	if (!s || !ie) {
		return;
	}

	/* Apply only when we are in FIXED mode (your current concept) */
	if (s->mac_sched.mode != DECT_MAC_SCHED_FIXED) {
		return;
	}

	/* Clamp max_pts to our local table size */
	uint8_t max_pts = ie->max_pts;
	if (max_pts == 0U) {
		return;
	}
	if (max_pts > DECT_MAX_PTS) {
		max_pts = DECT_MAX_PTS;
	}

	/* Store advertised max_pts/active_pts if you want */
	s->mac_sched.max_pts = max_pts;

	/* Copy allocation into existing scheduler table:
	 * Your settings table uses start_subslot/end_subslot (0..47).
	 */
	for (uint8_t i = 0; i < max_pts; i++) {
		uint8_t start_slot = ie->pt[i].start_slot;
		uint8_t end_slot   = ie->pt[i].end_slot;

		/* Defensive clamp */
		const uint8_t last_slot = (uint8_t)(DECT_RADIO_FRAME_SLOT_COUNT - 1U);
		if (start_slot > last_slot) start_slot = last_slot;
		if (end_slot > last_slot)   end_slot = last_slot;
		if (end_slot < start_slot) {
			end_slot = start_slot;
		}

		s->mac_sched.pt_slots[i].start_subslot = slot_to_start_subslot(start_slot);
		s->mac_sched.pt_slots[i].end_subslot   = slot_to_end_subslot(end_slot);
	}

	/* Optional debug */
	/* desh_info("FixedSched IE applied: max_pts=%u", max_pts); */
}