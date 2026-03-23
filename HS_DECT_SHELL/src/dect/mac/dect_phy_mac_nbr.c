/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>

#include "desh_print.h"

#include "dect_common.h"
#include "dect_app_time.h"
#include "dect_phy_common.h"
#include "dect_phy_mac_cluster_beacon.h"
#include "dect_phy_mac_nbr_bg_scan.h"
#include "dect_phy_mac_nbr.h"

struct dect_phy_mac_nbr_info_list_item nbrs[DECT_PHY_MAC_MAX_NEIGBORS] = {0};

static struct k_spinlock nbr_list_lock;

bool dect_phy_mac_nbr_info_get_by_long_rd_id(uint32_t long_rd_id,
					     struct dect_phy_mac_nbr_info_list_item *out)
{
	bool found = false;

	if (out == NULL) {
		return false;
	}

	k_spinlock_key_t key = k_spin_lock(&nbr_list_lock);

	for (int i = 0; i < DECT_PHY_MAC_MAX_NEIGBORS; i++) {
		if (nbrs[i].reserved && nbrs[i].long_rd_id == long_rd_id) {
			*out = nbrs[i];
			found = true;
			break;
		}
	}

	k_spin_unlock(&nbr_list_lock, key);
	return found;
}


bool dect_phy_mac_nbr_info_remove_by_long_rd_id(uint32_t long_rd_id)
{
	bool removed = false;

	k_spinlock_key_t key = k_spin_lock(&nbr_list_lock);

	for (int i = 0; i < DECT_PHY_MAC_MAX_NEIGBORS; i++) {
		if (nbrs[i].reserved && nbrs[i].long_rd_id == long_rd_id) {
			memset(&nbrs[i], 0, sizeof(nbrs[i]));
			removed = true;
			break;
		}
	}

	k_spin_unlock(&nbr_list_lock, key);
	return removed;
}

void dect_phy_mac_nbr_info_clear_all(void)
{
	k_spinlock_key_t key = k_spin_lock(&nbr_list_lock);
	memset(nbrs, 0, sizeof(nbrs));
	k_spin_unlock(&nbr_list_lock, key);
}

bool dect_phy_mac_nbr_info_store_n_update(uint64_t const *rcv_time, uint16_t channel,
					  uint32_t nw_id_24msb, uint8_t nw_id_8lsb,
					  uint32_t long_rd_id, uint16_t short_rd_id,
					  dect_phy_mac_cluster_beacon_t *beacon_msg,
					  dect_phy_mac_random_access_resource_ie_t *ra_ie,
					  bool print_update)
{
	bool done = true;
	bool inserted = false;
	bool updated = false;
	int64_t time_shift_mdm_ticks = 0;
	uint64_t stored_time = 0;
	uint16_t stored_channel = channel;

	if (rcv_time == NULL || beacon_msg == NULL || ra_ie == NULL) {
		return false;
	}

	k_spinlock_key_t key = k_spin_lock(&nbr_list_lock);

	int free_idx = -1;
	int found_idx = -1;

	for (int i = 0; i < DECT_PHY_MAC_MAX_NEIGBORS; i++) {
		if (nbrs[i].reserved) {
			if (nbrs[i].long_rd_id == long_rd_id) {
				found_idx = i;
				break;
			}
		} else if (free_idx < 0) {
			free_idx = i;
		}
	}

	if (found_idx >= 0) {
		time_shift_mdm_ticks =
			dect_phy_mac_cluster_beacon_rcv_time_shift_calculate(
				nbrs[found_idx].beacon_msg.cluster_beacon_period,
				nbrs[found_idx].time_rcvd_mdm_ticks,
				*rcv_time);

		if (channel) {
			nbrs[found_idx].channel = channel;
		}
		nbrs[found_idx].short_rd_id = short_rd_id;
		nbrs[found_idx].nw_id_24msb = nw_id_24msb;
		nbrs[found_idx].nw_id_8lsb = nw_id_8lsb;
		nbrs[found_idx].nw_id_32bit = ((nw_id_24msb << 8) | nw_id_8lsb);
		nbrs[found_idx].time_rcvd_mdm_ticks = *rcv_time;
		nbrs[found_idx].time_rcvd_shift_mdm_ticks = time_shift_mdm_ticks;
		nbrs[found_idx].beacon_msg = *beacon_msg;
		nbrs[found_idx].ra_ie = *ra_ie;

		stored_time = nbrs[found_idx].time_rcvd_mdm_ticks;
		stored_channel = nbrs[found_idx].channel;
		updated = true;
	} else if (free_idx >= 0) {
		nbrs[free_idx].reserved = true;
		nbrs[free_idx].channel = channel;
		nbrs[free_idx].long_rd_id = long_rd_id;
		nbrs[free_idx].short_rd_id = short_rd_id;
		nbrs[free_idx].nw_id_24msb = nw_id_24msb;
		nbrs[free_idx].nw_id_8lsb = nw_id_8lsb;
		nbrs[free_idx].nw_id_32bit = ((nw_id_24msb << 8) | nw_id_8lsb);
		nbrs[free_idx].time_rcvd_mdm_ticks = *rcv_time;
		nbrs[free_idx].time_rcvd_shift_mdm_ticks = 0;
		nbrs[free_idx].beacon_msg = *beacon_msg;
		nbrs[free_idx].ra_ie = *ra_ie;

		stored_time = nbrs[free_idx].time_rcvd_mdm_ticks;
		stored_channel = nbrs[free_idx].channel;
		inserted = true;
	} else {
		done = false;
	}

	k_spin_unlock(&nbr_list_lock, key);

	if (!done) {
		desh_error("%s: cannot store scanning nbr result for long rd id %u",
			   __func__, long_rd_id);
		return false;
	}

	if (updated) {
		dect_phy_mac_nbr_bg_scan_rcv_time_shift_update(
			long_rd_id, *rcv_time, time_shift_mdm_ticks);
	}

	if (inserted) {
		desh_print("Neighbor with long rd id %u (0x%08x), short rd id %u (0x%04x), "
			   "channel %d, stored to nbr list.",
			   long_rd_id, long_rd_id, short_rd_id, short_rd_id, stored_channel);
	} else if (updated && print_update) {
		desh_print("Neighbor with long rd id %u (0x%08x), short rd id %u (0x%04x), "
			   "nw (24bit MSB: %u (0x%06x), 8bit LSB: %u (0x%02x)), channel %d\n"
			   "  updated with time %llu (time shift %lld mdm ticks) to nbr list.",
			   long_rd_id, long_rd_id, short_rd_id, short_rd_id, nw_id_24msb,
			   nw_id_24msb, nw_id_8lsb, nw_id_8lsb, stored_channel,
			   stored_time, time_shift_mdm_ticks);
	}

	return true;
}

void dect_phy_mac_nbr_status_print(void)
{
	uint64_t time_now = dect_app_modem_time_now();
	struct dect_phy_mac_nbr_info_list_item local[DECT_PHY_MAC_MAX_NEIGBORS];

	k_spinlock_key_t key = k_spin_lock(&nbr_list_lock);
	memcpy(local, nbrs, sizeof(local));
	k_spin_unlock(&nbr_list_lock, key);

	desh_print("Neighbor list status:");
	for (int i = 0; i < DECT_PHY_MAC_MAX_NEIGBORS; i++) {
		if (local[i].reserved) {
			int64_t time_from_last_received_ms =
				MODEM_TICKS_TO_MS(time_now - local[i].time_rcvd_mdm_ticks);

			desh_print("  Neighbor %d:", i + 1);
			desh_print("   network ID (24bit MSB): %u (0x%06x)",
				   local[i].nw_id_24msb, local[i].nw_id_24msb);
			desh_print("   network ID (8bit LSB):  %u (0x%02x)",
				   local[i].nw_id_8lsb, local[i].nw_id_8lsb);
			desh_print("   network ID (32bit):     %u (0x%06x)",
				   local[i].nw_id_32bit, local[i].nw_id_32bit);
			desh_print("   long RD ID:             %u", local[i].long_rd_id);
			desh_print("   short RD ID:            %u", local[i].short_rd_id);
			desh_print("   channel:                %u", local[i].channel);
			desh_print("   Last seen:              %d msecs ago", time_from_last_received_ms);
			dect_phy_mac_nbr_bg_scan_status_print_for_target_long_rd_id(
				local[i].long_rd_id);
		}
	}
}

bool dect_phy_mac_nbr_is_in_channel(uint16_t channel)
{
	bool found = false;

	k_spinlock_key_t key = k_spin_lock(&nbr_list_lock);

	for (int i = 0; i < DECT_PHY_MAC_MAX_NEIGBORS; i++) {
		if (nbrs[i].reserved && nbrs[i].channel == channel) {
			found = true;
			break;
		}
	}

	k_spin_unlock(&nbr_list_lock, key);
	return found;
}