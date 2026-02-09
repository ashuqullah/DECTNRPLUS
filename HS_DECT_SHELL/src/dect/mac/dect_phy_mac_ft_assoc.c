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

/* ISR-safe lock */
static struct k_spinlock g_tab_lock;

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
	int assoc_count = dect_phy_mac_ft_assoc_count_get();
	struct dect_phy_settings *s = dect_common_settings_ref_get();

	if (assoc_count >= s->mac_sched.max_pts) {
		desh_warn("FT: association rejected, max PTs reached (%d)",
				s->mac_sched.max_pts);
		return -ENOMEM;
	}

	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);

	int lim = tab_limit_get();

	/* update if exists */
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used && g_tab[i].pt_long_rd_id == pt_long_rd_id) {
			g_tab[i].pt_short_rd_id = pt_short_rd_id;
			g_tab[i].last_rssi = rssi;
			g_tab[i].last_seen_bb = t_bb;
			k_spin_unlock(&g_tab_lock, key);
			return (i + 1); /* PT index = slot in table + 1 */
		}
	}

	/* insert to first free slot => assigns PT index */
	for (int i = 0; i < lim; i++) {
		if (!g_tab[i].used) {
			g_tab[i].used = true;
			g_tab[i].pt_long_rd_id = pt_long_rd_id;
			g_tab[i].pt_short_rd_id = pt_short_rd_id;
			g_tab[i].last_rssi = rssi;
			g_tab[i].last_seen_bb = t_bb;
			k_spin_unlock(&g_tab_lock, key);
			return (i + 1);
		}
	}

	k_spin_unlock(&g_tab_lock, key);
	return -ENOMEM; /* FT full */
}

bool dect_phy_mac_ft_assoc_is_associated(uint32_t pt_long_rd_id)
{
	bool ok = false;

	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);

	int lim = tab_limit_get();
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used && g_tab[i].pt_long_rd_id == pt_long_rd_id) {
			ok = true;
			break;
		}
	}

	k_spin_unlock(&g_tab_lock, key);
	return ok;
}

int dect_phy_mac_ft_assoc_pt_index_get(uint32_t pt_long_rd_id, uint8_t *pt_index_out)
{
	if (!pt_index_out || pt_long_rd_id == 0) {
		return -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);

	int lim = tab_limit_get();
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used && g_tab[i].pt_long_rd_id == pt_long_rd_id) {
			*pt_index_out = (uint8_t)(i + 1);
			k_spin_unlock(&g_tab_lock, key);
			return 0;
		}
	}

	k_spin_unlock(&g_tab_lock, key);
	return -ENOENT;
}

void dect_phy_mac_ft_assoc_clear_all(void)
{
	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);
	memset(g_tab, 0, sizeof(g_tab));
	k_spin_unlock(&g_tab_lock, key);

	/* Called from shell context -> safe to print */
	desh_print("FT assoc table cleared.");
}

/// @brief 
/// @param pt_long_rd_id 
/// @return 
int dect_phy_mac_ft_assoc_remove(uint32_t pt_long_rd_id)
{
	if (pt_long_rd_id == 0) {
		return -EINVAL;
	}

	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);

	int lim = tab_limit_get();
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used && g_tab[i].pt_long_rd_id == pt_long_rd_id) {
			memset(&g_tab[i], 0, sizeof(g_tab[i]));
			k_spin_unlock(&g_tab_lock, key);
			return 0;
		}
	}

	k_spin_unlock(&g_tab_lock, key);
	return -ENOENT;
}

int dect_phy_mac_ft_assoc_count_get(void)
{
	int cnt = 0;

	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);

	int lim = tab_limit_get();
	for (int i = 0; i < lim; i++) {
		if (g_tab[i].used) {
			cnt++;
		}
	}

	k_spin_unlock(&g_tab_lock, key);
	return cnt;
}


void dect_phy_mac_ft_assoc_status_print(void)
{
	struct dect_phy_settings *s = dect_common_settings_ref_get();

	k_spinlock_key_t key = k_spin_lock(&g_tab_lock);

	int lim = tab_limit_get();
	desh_print("FT assoc table (max=%d):", lim);

	for (int i = 0; i < lim; i++) {
		if (!g_tab[i].used) {
			continue;
		}

		uint16_t ss = 0, es = 0;
		if (s->mac_sched.mode == DECT_MAC_SCHED_FIXED && i < DECT_MAX_PTS) {
			ss = s->mac_sched.pt_slots[i].start_subslot;
			es = s->mac_sched.pt_slots[i].end_subslot;
		}

		desh_print("  PT%d long=%u short=%u rssi=%d last_seen_bb=%llu slots=%u:%u",
			   i + 1,
			   g_tab[i].pt_long_rd_id,
			   g_tab[i].pt_short_rd_id,
			   g_tab[i].last_rssi,
			   (unsigned long long)g_tab[i].last_seen_bb,
			   ss, es);
	}

	k_spin_unlock(&g_tab_lock, key);
}
