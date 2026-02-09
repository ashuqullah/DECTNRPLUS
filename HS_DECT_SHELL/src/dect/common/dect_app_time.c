/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <nrf_modem_dect_phy.h>

#include "dect_app_time.h"

static uint64_t m_last_modem_time_bb;
static uint64_t m_last_app_time_bb;

/* Current app processor time in modem bb ticks. */
static uint64_t dect_app_ztime_now_in_mdm_ticks(void)
{
	uint64_t time_now_ns = k_ticks_to_ns_floor64(sys_clock_tick_get());

	return time_now_ns / 1000 * NRF_MODEM_DECT_MODEM_TIME_TICK_RATE_KHZ / 1000;
}

/* Current time in modem ticks */
uint64_t dect_app_modem_time_now(void)
{
	uint64_t time_now;

	time_now = m_last_modem_time_bb + (dect_app_ztime_now_in_mdm_ticks() - m_last_app_time_bb);

	return time_now;
}

void dect_app_modem_time_save(uint64_t const *time)
{
	if (*time > m_last_modem_time_bb) {
		m_last_modem_time_bb = *time;
		m_last_app_time_bb = dect_app_ztime_now_in_mdm_ticks();
	}
}
/*Convert aptime modem ticks to us micro second*/

uint64_t dect_app_time_us_to_mdm_ticks(uint32_t us)
{
	/* NRF_MODEM_DECT_MODEM_TIME_TICK_RATE_KHZ = ticks per millisecond */
	/* ticks = us * (ticks/ms) / 1000 */
	return ((uint64_t)us * (uint64_t)NRF_MODEM_DECT_MODEM_TIME_TICK_RATE_KHZ) / 1000ULL;
}

uint32_t dect_app_time_mdm_ticks_to_us(uint64_t ticks)
{
	/* us = ticks * 1000 / (ticks/ms) */
	return (uint32_t)((ticks * 1000ULL) / (uint64_t)NRF_MODEM_DECT_MODEM_TIME_TICK_RATE_KHZ);
}
/*==============================================================================================*/