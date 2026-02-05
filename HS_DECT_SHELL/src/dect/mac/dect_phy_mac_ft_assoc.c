#include <zephyr/kernel.h>
#include <string.h>

#include "desh_print.h"
#include "dect_common_settings.h"
#include "dect_phy_mac_ft_assoc.h"

struct ft_assoc_entry {
	bool used;
	uint32_t pt_long_rd_id;
	uint16_t pt_short_rd_id;
	int8_t last_rssi;
	uint64_t last_seen_bb;
};

static struct ft_assoc_entry g_tab[DECT_MAX_PTS];
static K_MUTEX_DEFINE(g_tab_mutex);

static int tab_limit_get(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();
	int lim = (s->mac_sched.max_pts > 0) ? s->mac_sched.max_pts : 1;
	if (lim > DECT_MAX_PTS) {
		lim = DECT_MAX_PTS;
	}
	return lim;
}

int dect_phy_mac_ft_assoc_add_or_update(uint32_t pt_long_rd_id,
				       uint16_t pt_short_rd_id,
				       int8_t rssi,
				       uint64_t t_bb)
{
	if (pt_long_rd_id == 0) {
		return -EINVAL;
	}

	k_mutex_lock(&g_tab_mutex, K_FOREVER);

	int lim = tab_limit_get();

	/* update if exists */
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used && g_tab[i].pt_long_rd_id == pt_long_rd_id) {
			g_tab[i].pt_short_rd_id = pt_short_rd_id;
			g_tab[i].last_rssi = rssi;
			g_tab[i].last_seen_bb = t_bb;
			k_mutex_unlock(&g_tab_mutex);
			return 0;
		}
	}

	/* insert to first free slot */
	for (int i = 0; i < lim; i++) {
		if (!g_tab[i].used) {
			g_tab[i].used = true;
			g_tab[i].pt_long_rd_id = pt_long_rd_id;
			g_tab[i].pt_short_rd_id = pt_short_rd_id;
			g_tab[i].last_rssi = rssi;
			g_tab[i].last_seen_bb = t_bb;
			k_mutex_unlock(&g_tab_mutex);
			return 0;
		}
	}

	k_mutex_unlock(&g_tab_mutex);
	return -ENOMEM;
}

bool dect_phy_mac_ft_assoc_is_associated(uint32_t pt_long_rd_id)
{
	bool ok = false;

	k_mutex_lock(&g_tab_mutex, K_FOREVER);

	int lim = tab_limit_get();
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used && g_tab[i].pt_long_rd_id == pt_long_rd_id) {
			ok = true;
			break;
		}
	}

	k_mutex_unlock(&g_tab_mutex);
	return ok;
}

void dect_phy_mac_ft_assoc_clear_all(void)
{
	k_mutex_lock(&g_tab_mutex, K_FOREVER);
	memset(g_tab, 0, sizeof(g_tab));
	k_mutex_unlock(&g_tab_mutex);

	desh_print("FT assoc table cleared.");
}

void dect_phy_mac_ft_assoc_status_print(void)
{
	k_mutex_lock(&g_tab_mutex, K_FOREVER);

	int lim = tab_limit_get();
	desh_print("FT assoc table (max=%d):", lim);

	for (int i = 0; i < lim; i++) {
		if (!g_tab[i].used) {
			continue;
		}
		desh_print("  [%d] PT long=%u short=%u rssi=%d last_seen_bb=%llu",
			   i,
			   g_tab[i].pt_long_rd_id,
			   g_tab[i].pt_short_rd_id,
			   g_tab[i].last_rssi,
			   (unsigned long long)g_tab[i].last_seen_bb);
	}

	k_mutex_unlock(&g_tab_mutex);
}
