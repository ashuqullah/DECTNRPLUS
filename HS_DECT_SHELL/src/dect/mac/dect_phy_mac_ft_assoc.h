#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Add/update PT entry. Returns PT index (1..max_pts) on success, negative errno on failure. */
int dect_phy_mac_ft_assoc_add_or_update(uint32_t pt_long_rd_id,
				       uint16_t pt_short_rd_id,
				       int8_t rssi,
				       uint64_t t_bb);

bool dect_phy_mac_ft_assoc_is_associated(uint32_t pt_long_rd_id);

/* Get assigned PT index (1..max_pts). */
int dect_phy_mac_ft_assoc_pt_index_get(uint32_t pt_long_rd_id, uint8_t *pt_index_out);

/* Remove one PT from the FT table. */
int dect_phy_mac_ft_assoc_remove(uint32_t pt_long_rd_id);

/* Clear all PTs from FT table. */
void dect_phy_mac_ft_assoc_clear_all(void);

/* Print FT assoc table. */
void dect_phy_mac_ft_assoc_status_print(void);

/* Return number of currently used entries (0..max_pts). */
int dect_phy_mac_ft_assoc_count_get(void);
