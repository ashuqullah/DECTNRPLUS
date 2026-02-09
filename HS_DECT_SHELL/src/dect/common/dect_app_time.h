/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef DECT_APP_TIME_H
#define DECT_APP_TIME_H

uint64_t dect_app_modem_time_now(void);
void dect_app_modem_time_save(uint64_t const *time);

/*Convert aptime modem ticks to us micro second*/

uint64_t dect_app_time_us_to_mdm_ticks(uint32_t us);
uint32_t dect_app_time_mdm_ticks_to_us(uint64_t ticks);
/* =====================================================*/

#endif /* DECT_APP_TIME_H */
