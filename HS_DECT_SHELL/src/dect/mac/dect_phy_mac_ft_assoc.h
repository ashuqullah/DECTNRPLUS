#pragma once

#include <stdint.h>
#include <stdbool.h>

int dect_phy_mac_ft_assoc_add_or_update(uint32_t pt_long_rd_id,
				       uint16_t pt_short_rd_id,
				       int8_t rssi,
				       uint64_t t_bb);

bool dect_phy_mac_ft_assoc_is_associated(uint32_t pt_long_rd_id);

void dect_phy_mac_ft_assoc_clear_all(void);

void dect_phy_mac_ft_assoc_status_print(void);
