/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/sys/byteorder.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "desh_print.h"

#include "dect_common.h"
#include "dect_phy_common.h"
#include "dect_common_utils.h"
#include "dect_common_settings.h"

#include "dect_phy_api_scheduler.h"
#include "dect_phy_ctrl.h"
#include "dect_phy_scan.h"
#include "dect_phy_rx.h"

#include "dect_phy_mac_cluster_beacon.h"

#include "dect_phy_mac_common.h"
#include "dect_phy_mac_pdu.h"
#include "dect_phy_mac_ctrl.h"
#include "dect_phy_mac_ft_assoc.h"
#include "dect_app_time.h"

/*=============================Constant Fixed Scheduall  ===========================================*/
#define HS_DECT_IE_EXT_TYPE_SCHED_ASSIGN  0xA1

/* HS_DECT: Vendor-specific beacon payload carried via IE_TYPE_ESCAPE */
#define HS_DECT_BEACON_MAGIC0        0x48 /* 'H' */
#define HS_DECT_BEACON_MAGIC1        0x53 /* 'S' */
#define HS_DECT_BEACON_VER           0x01

#define HS_DECT_BEACON_SCHED_RANDOM  0x00
#define HS_DECT_BEACON_SCHED_FIXED   0x01
#define HS_DECT_IE_EXT_TYPE_FT_MODE  0xA2

/* Payload layout (8 bytes):
 * [0]='H' [1]='S' [2]=ver [3]=sched_mode [4]=role_bitmap [5]=max_pts [6]=slots_per_frame [7]=reserved
 */
#define HS_DECT_BEACON_PAYLOAD_LEN   8
#define HS_DECT_SLOTS_PER_FRAME      24  /* keep consistent with your project */

/*========================================================================*/
static void hs_sdu_list_free_all(sys_dlist_t *list)
{
	while (!sys_dlist_is_empty(list)) {
		sys_dnode_t *n = sys_dlist_get(list);
		if (n) {
			dect_phy_mac_sdu_t *item = CONTAINER_OF(n, dect_phy_mac_sdu_t, dnode);
			k_free(item);
		}
	}
}

static struct dect_phy_mac_cluster_beacon_data {
	bool running;

	uint8_t next_sfn;

	uint8_t encoded_cluster_beacon_pdu[DECT_DATA_MAX_LEN];
	uint16_t encoded_cluster_beacon_pdu_len;

	dect_phy_mac_cluster_beacon_t last_cluster_beacon_msg;
	dect_phy_mac_random_access_resource_ie_t last_rach_ie;
	uint64_t last_tx_frame_time;

	struct dect_phy_mac_beacon_start_params start_params;
} beacon_data;

struct dect_phy_mac_cluster_beacon_lms_rssi_scan_data {
	int8_t busy_rssi_limit;
	int8_t free_rssi_limit;

	/* Cluster reservations in symbol resolution within a frame */
	bool cluster_beacon_reserved_symbols_in_frame[DECT_RADIO_FRAME_SYMBOL_COUNT];
	bool cluster_ra_reserved_symbols_in_frame[DECT_RADIO_FRAME_SYMBOL_COUNT];

	enum dect_phy_rssi_scan_data_result_verdict
		scan_result_symbols_in_frame[DECT_RADIO_FRAME_SYMBOL_COUNT];
} lms_rssi_scan_data;

static void dect_phy_mac_cluster_beacon_scheduler_list_items_remove(void);


static int dect_phy_mac_cluster_beacon_encode(struct dect_phy_mac_beacon_start_params *params,
					     uint8_t **target_ptr,
					     union nrf_modem_dect_phy_hdr *out_phy_header)
{
	struct dect_phy_settings *current_settings = dect_common_settings_ref_get();

	if (current_settings == NULL || params == NULL || target_ptr == NULL ||
	    *target_ptr == NULL || out_phy_header == NULL) {
		return -EINVAL;
	}

	/* ---------- PHY header (type1) ---------- */
	uint8_t phy_tx_power = dect_common_utils_dbm_to_phy_tx_power(params->tx_power_dbm);

	struct dect_phy_ctrl_field_common header = {
		.packet_length_type = DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS,
		.header_format = DECT_PHY_HEADER_TYPE1,
		.short_network_id = (uint8_t)(current_settings->common.network_id & 0xFF),
		.transmitter_identity_hi = (uint8_t)(current_settings->common.short_rd_id >> 8),
		.transmitter_identity_lo = (uint8_t)(current_settings->common.short_rd_id & 0xFF),
		.df_mcs = 0,
		.transmit_power = phy_tx_power,
	};

	/* ---------- MAC headers ---------- */
	dect_phy_mac_type_header_t type_header = {
		.version = 0,
		.security = 0,
		.type = DECT_PHY_MAC_HEADER_TYPE_BEACON,
	};

	dect_phy_mac_common_header_t common_header = {
		.type = DECT_PHY_MAC_HEADER_TYPE_BEACON,
		.reset = 1,
		.seq_nbr = 0,
		.nw_id = ((current_settings->common.network_id >> 8) & DECT_COMMON_UTILS_BIT_MASK_24BIT),
		.transmitter_id = current_settings->common.transmitter_id,
	};

	/* ---------- SDU list ---------- */
	sys_dlist_t sdu_list;
	sys_dlist_init(&sdu_list);

	/* Cluster Beacon SDU */
	dect_phy_mac_sdu_t *cluster_beacon_sdu = k_calloc(1, sizeof(*cluster_beacon_sdu));
	if (!cluster_beacon_sdu) {
		return -ENOMEM;
	}

	cluster_beacon_sdu->mux_header.mac_ext = DECT_PHY_MAC_EXT_8BIT_LEN;
	cluster_beacon_sdu->mux_header.ie_type = DECT_PHY_MAC_IE_TYPE_CLUSTER_BEACON;
	cluster_beacon_sdu->mux_header.payload_length = 5;

	cluster_beacon_sdu->message_type = DECT_PHY_MAC_MESSAGE_TYPE_CLUSTER_BEACON;
	cluster_beacon_sdu->message.cluster_beacon.system_frame_number = beacon_data.next_sfn;
	cluster_beacon_sdu->message.cluster_beacon.tx_pwr_bit = 1;
	cluster_beacon_sdu->message.cluster_beacon.pwr_const_bit = 0;
	cluster_beacon_sdu->message.cluster_beacon.frame_offset_bit = 0;
	cluster_beacon_sdu->message.cluster_beacon.next_channel_bit = 0;
	cluster_beacon_sdu->message.cluster_beacon.time_to_next = 0;
	cluster_beacon_sdu->message.cluster_beacon.nw_beacon_period = DECT_PHY_MAC_NW_BEACON_PERIOD_50MS;
	cluster_beacon_sdu->message.cluster_beacon.cluster_beacon_period = DECT_PHY_MAC_CLUSTER_BEACON_PERIOD_2000MS;
	cluster_beacon_sdu->message.cluster_beacon.count_to_trigger = 0;
	cluster_beacon_sdu->message.cluster_beacon.relative_quality = 0;
	cluster_beacon_sdu->message.cluster_beacon.min_quality = 0;
	cluster_beacon_sdu->message.cluster_beacon.max_phy_tx_power =
		dect_common_utils_dbm_to_phy_tx_power(19);

	sys_dlist_append(&sdu_list, &cluster_beacon_sdu->dnode);

	/* Random Access Resource IE SDU */
	dect_phy_mac_sdu_t *ra_ie_sdu = k_calloc(1, sizeof(*ra_ie_sdu));
	if (!ra_ie_sdu) {
		hs_sdu_list_free_all(&sdu_list);
		return -ENOMEM;
	}

	ra_ie_sdu->mux_header.mac_ext = DECT_PHY_MAC_EXT_8BIT_LEN;
	ra_ie_sdu->mux_header.ie_type = DECT_PHY_MAC_IE_TYPE_RANDOM_ACCESS_RESOURCE_IE;
	ra_ie_sdu->mux_header.payload_length = 7;

	ra_ie_sdu->message_type = DECT_PHY_MAC_MESSAGE_RANDOM_ACCESS_RESOURCE_IE;
	ra_ie_sdu->message.rach_ie.channel_included = false;
	ra_ie_sdu->message.rach_ie.channel2_included = false;
	ra_ie_sdu->message.rach_ie.sfn_included = false;
	ra_ie_sdu->message.rach_ie.length_type = DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS;
	ra_ie_sdu->message.rach_ie.length = DECT_PHY_MAC_CLUSTER_BEACON_RA_LENGTH_SLOTS;
	ra_ie_sdu->message.rach_ie.start_subslot = DECT_PHY_MAC_CLUSTER_BEACON_RA_START_SUBSLOT;
	ra_ie_sdu->message.rach_ie.max_rach_length_type = DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS;
	ra_ie_sdu->message.rach_ie.max_rach_length = 4;
	ra_ie_sdu->message.rach_ie.dect_delay = 1;
	ra_ie_sdu->message.rach_ie.response_win = 10;
	ra_ie_sdu->message.rach_ie.validity = DECT_PHY_MAC_CLUSTER_BEACON_RA_VALIDITY;
	ra_ie_sdu->message.rach_ie.repeat = DECT_PHY_MAC_RA_REPEAT_TYPE_FRAMES;
	ra_ie_sdu->message.rach_ie.repetition = DECT_PHY_MAC_CLUSTER_BEACON_RA_REPETITION;
	ra_ie_sdu->message.rach_ie.cw_min_sig = 0;
	ra_ie_sdu->message.rach_ie.cw_max_sig = 7;

	sys_dlist_append(&sdu_list, &ra_ie_sdu->dnode);

	/* ================= HS_DECT: advertise scheduling mode ================= */
	{
		uint8_t payload[2];
		payload[0] = HS_DECT_ASSOC_EXT_VER;
		payload[1] = (current_settings->mac_sched.mode == DECT_MAC_SCHED_FIXED) ? 1 : 0;

		dect_phy_mac_sdu_t *mode_sdu = k_calloc(1, sizeof(*mode_sdu));
		if (mode_sdu) {
			mode_sdu->mux_header.mac_ext = DECT_PHY_MAC_EXT_16BIT_LEN;
			mode_sdu->mux_header.ie_type = DECT_PHY_MAC_IE_TYPE_EXTENSION;
			mode_sdu->mux_header.ie_ext = HS_DECT_IE_EXT_TYPE_ASSOC_POLICY;
			mode_sdu->mux_header.payload_length = sizeof(payload);

			mode_sdu->message_type = DECT_PHY_MAC_MESSAGE_ESCAPE;
			mode_sdu->message.common_msg.data_length = sizeof(payload);
			memcpy(mode_sdu->message.common_msg.data, payload, sizeof(payload));

			sys_dlist_append(&sdu_list, &mode_sdu->dnode);
		}
	}
	/* ===================================================================== */

	/* ---------- Encode MAC PDU ---------- */
	uint8_t *pdu_ptr = *target_ptr;

	pdu_ptr = dect_phy_mac_pdu_type_header_encode(&type_header, pdu_ptr);
	if (pdu_ptr == NULL) {
		hs_sdu_list_free_all(&sdu_list);
		return -EINVAL;
	}

	pdu_ptr = dect_phy_mac_pdu_common_header_encode(&common_header, pdu_ptr);
	if (pdu_ptr == NULL) {
		hs_sdu_list_free_all(&sdu_list);
		return -EINVAL;
	}

	pdu_ptr = dect_phy_mac_pdu_sdus_encode(pdu_ptr, &sdu_list);
	if (pdu_ptr == NULL) {
		hs_sdu_list_free_all(&sdu_list);
		return -EINVAL;
	}

	/* ---------- Calculate lengths & padding (SAFE) ---------- */
	uint16_t encoded_pdu_length = (uint16_t)(pdu_ptr - *target_ptr);

	header.packet_length = dect_common_utils_phy_packet_length_calculate(
		encoded_pdu_length, header.packet_length_type, header.df_mcs);
	if ((int)header.packet_length <= 0) {
		hs_sdu_list_free_all(&sdu_list);
		return -EINVAL;
	}

	int16_t total_byte_count = dect_common_utils_slots_in_bytes(header.packet_length, header.df_mcs);
	if (total_byte_count <= 0) {
		hs_sdu_list_free_all(&sdu_list);
		return -EINVAL;
	}

	int16_t padding_need = total_byte_count - (int16_t)encoded_pdu_length;
	if (padding_need < 0) {
		desh_error("(%s): Beacon PDU too long: enc=%u bytes, slots=%d -> max=%d bytes",
			   __func__, encoded_pdu_length, header.packet_length, total_byte_count);
		hs_sdu_list_free_all(&sdu_list);
		return -EMSGSIZE;
	}

	if (padding_need > 0) {
		int err = dect_phy_mac_pdu_sdu_list_add_padding(&pdu_ptr, &sdu_list, padding_need);
		if (err) {
			desh_warn("(%s): Failed to add padding: err %d (continue)", __func__, err);
		}
	}

	*target_ptr = pdu_ptr;

	/* ---------- Output PHY header ---------- */
	union nrf_modem_dect_phy_hdr phy_header;
	memset(&phy_header, 0, sizeof(phy_header));
	memcpy(&phy_header.type_1, &header, sizeof(phy_header.type_1));
	memcpy(out_phy_header, &phy_header, sizeof(*out_phy_header));

	/* Save last beacon content (for debug) */
	beacon_data.last_cluster_beacon_msg = cluster_beacon_sdu->message.cluster_beacon;
	beacon_data.last_rach_ie = ra_ie_sdu->message.rach_ie;

	/* Free SDU nodes */
	hs_sdu_list_free_all(&sdu_list);

	return header.packet_length;
}


void dect_phy_mac_ctrl_lms_rssi_scan_data_init(int beacon_tx_slot_count)
{
	struct dect_phy_settings *current_settings = dect_common_settings_ref_get();

	memset(&lms_rssi_scan_data, 0, sizeof(lms_rssi_scan_data));

	lms_rssi_scan_data.busy_rssi_limit = current_settings->rssi_scan.busy_threshold;
	lms_rssi_scan_data.free_rssi_limit = current_settings->rssi_scan.free_threshold;

	int beacon_tx_symbols =
		beacon_tx_slot_count * DECT_RADIO_SLOT_SYMBOL_COUNT;

	for (int i = 0; i < beacon_tx_symbols && i < DECT_RADIO_FRAME_SYMBOL_COUNT; i++) {
		lms_rssi_scan_data.cluster_beacon_reserved_symbols_in_frame[i] = true;
	}

	int beacon_ra_start_symbols = (DECT_PHY_MAC_CLUSTER_BEACON_RA_START_SUBSLOT *
				       DECT_PHY_MAC_CLUSTER_BEACON_RA_LENGTH_SLOTS) - 1;
	int beacon_ra_end_symbols =
		beacon_ra_start_symbols +
		(DECT_PHY_MAC_CLUSTER_BEACON_RA_LENGTH_SLOTS * DECT_RADIO_SLOT_SYMBOL_COUNT);

	for (int i = beacon_ra_start_symbols;
	     i < beacon_ra_end_symbols && i < DECT_RADIO_FRAME_SYMBOL_COUNT; i++) {
		lms_rssi_scan_data.cluster_ra_reserved_symbols_in_frame[i] = true;
	}
}

void dect_phy_mac_ctrl_cluster_beacon_phy_api_direct_rssi_cb(
	const struct nrf_modem_dect_phy_rssi_event *p_meas_results)
{
	/* Handle Last Minute Scan results. This assumes that start time was frame start. */
	bool busy_in_beacon_tx = false;
	bool busy_in_rach = false;

	for (int i = 0; i < p_meas_results->meas_len && i < DECT_RADIO_FRAME_SYMBOL_COUNT; i++) {
		int8_t curr_meas = p_meas_results->meas[i];
		enum dect_phy_rssi_scan_data_result_verdict current_verdict;

		if (curr_meas < 0) {
			if (curr_meas > lms_rssi_scan_data.busy_rssi_limit) {
				current_verdict = DECT_PHY_RSSI_SCAN_VERDICT_BUSY;
			} else if (curr_meas <= lms_rssi_scan_data.free_rssi_limit) {
				current_verdict = DECT_PHY_RSSI_SCAN_VERDICT_FREE;
			} else {
				current_verdict = DECT_PHY_RSSI_SCAN_VERDICT_POSSIBLE;
			}
			lms_rssi_scan_data.scan_result_symbols_in_frame[i] = current_verdict;

			if (current_verdict == DECT_PHY_RSSI_SCAN_VERDICT_BUSY) {
				if (lms_rssi_scan_data
					.cluster_beacon_reserved_symbols_in_frame[i]) {
					busy_in_beacon_tx = true;
				} else if (lms_rssi_scan_data
						.cluster_ra_reserved_symbols_in_frame[i]) {
					busy_in_rach = true;
				}
			}
		}
	}

	/* As a consequence of the LMS, we are simply stopping beacon if detecting BUSY in
	 * beacon TX or in RACH subslots.
	 */
	if (busy_in_beacon_tx) {
		/* Remove beacon tx ASAP */
		dect_phy_api_scheduler_th_list_item_remove_dealloc_by_phy_op_handle(
			DECT_PHY_MAC_BEACON_TX_HANDLE);
		dect_phy_mac_ctrl_cluster_beacon_stop(
			DECT_PHY_MAC_CTRL_BEACON_STOP_CAUSE_LMS_BEACON_TX);
	} else  if (busy_in_rach) {
		dect_phy_mac_ctrl_cluster_beacon_stop(
			DECT_PHY_MAC_CTRL_BEACON_STOP_CAUSE_LMS_RACH);
	}
}

static void dect_phy_mac_cluster_beacon_to_mdm_cb(
	struct dect_phy_common_op_completed_params *params, uint64_t frame_time)
{
	if (params->status == NRF_MODEM_DECT_PHY_SUCCESS) {
		beacon_data.last_tx_frame_time = frame_time;
	}
}

uint64_t dect_phy_mac_cluster_beacon_last_tx_frame_time_get(void)
{
	return beacon_data.last_tx_frame_time;
}

int dect_phy_mac_cluster_beacon_tx_start(struct dect_phy_mac_beacon_start_params *params)
{

	if (beacon_data.running) {
		desh_error("(%s): Beacon already running", __func__);
		return -EINVAL;
	}
	struct dect_phy_settings *current_settings = dect_common_settings_ref_get();
	uint32_t interval_mdm_ticks = MS_TO_MODEM_TICKS(DECT_PHY_MAC_CLUSTER_BEACON_INTERVAL_MS);
	uint8_t encoded_beacon_pdu[DECT_DATA_MAX_LEN];
	union nrf_modem_dect_phy_hdr phy_header;
	uint8_t slot_count = 0;
	uint8_t *pdu_ptr = encoded_beacon_pdu;
	int ret;

	memset(encoded_beacon_pdu, 0, DECT_DATA_MAX_LEN);
	memset(&beacon_data, 0, sizeof(struct dect_phy_mac_cluster_beacon_data));

	/* Encode cluster beacon */
	ret = dect_phy_mac_cluster_beacon_encode(params, &pdu_ptr, &phy_header);
	if (ret < 0) {
		desh_error("(%s): Failed to encode beacon", __func__);
		return ret;
	}
	slot_count = ret + 1;
	beacon_data.start_params = *params;

	dect_phy_mac_ctrl_lms_rssi_scan_data_init(slot_count);

	/* Schedule beaconing */
	uint64_t first_possible_tx;
	uint64_t time_now = dect_app_modem_time_now();
	uint64_t start_time = 0;

	first_possible_tx = time_now + (SECONDS_TO_MODEM_TICKS(1));
	start_time = first_possible_tx;

	/* Schedule Last Minute Scannings (LMS) before beacon TX and
	 * announcing random access resources.
	 * Due to scheduling delays and lack of stopping of a scheduled TX operation in phy api.
	 * LMS is done 2 frames before beacon TX but all slots at that frame are covered where
	 * we are TX/RA.
	 * Note: this is not exactly compliant with the LMS in MAC spec (which requires
	 * the scan to be 1 and/or 0.5 frames before resources being announced or own transmission).
	 */
	struct dect_phy_api_scheduler_list_item_config *rssi_list_item_conf;
	struct dect_phy_api_scheduler_list_item *rssi_list_item =
		dect_phy_api_scheduler_list_item_alloc_rssi_element(&rssi_list_item_conf);

	if (!rssi_list_item) {
		desh_error("(%s): dect_phy_api_scheduler_list_item_alloc_tx_element failed: No "
			   "memory to TX a beacon");
		return -ENOMEM;
	}
	rssi_list_item->phy_op_handle = DECT_PHY_MAC_BEACON_LMS_RSSI_SCAN_HANDLE;

	rssi_list_item_conf->channel = params->beacon_channel;
	rssi_list_item_conf->frame_time =
		start_time - (2 * DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS);
	rssi_list_item_conf->start_slot = 0;

	/* Let it run in intervals in a scheduler */
	rssi_list_item_conf->interval_mdm_ticks = interval_mdm_ticks;
	rssi_list_item_conf->rssi.rssi_op_params.start_time = rssi_list_item_conf->frame_time;
	rssi_list_item_conf->rssi.rssi_op_params.handle = rssi_list_item->phy_op_handle;
	rssi_list_item_conf->rssi.rssi_op_params.carrier = rssi_list_item_conf->channel;
	rssi_list_item_conf->rssi.rssi_op_params.duration = DECT_RADIO_FRAME_SUBSLOT_COUNT;
	rssi_list_item_conf->rssi.rssi_op_params.reporting_interval =
		NRF_MODEM_DECT_PHY_RSSI_INTERVAL_24_SLOTS;

	/* Add RSSI measurement operation to scheduler list */
	if (!dect_phy_api_scheduler_list_item_add(rssi_list_item)) {
		desh_error("(%s): dect_phy_api_scheduler_list_item_add failed for RSSI "
			   "measurement -- continue",
			(__func__));
		dect_phy_api_scheduler_list_item_dealloc(rssi_list_item);
	}

	struct dect_phy_api_scheduler_list_item_config *sched_list_item_conf;
	struct dect_phy_api_scheduler_list_item *sched_list_item =
		dect_phy_api_scheduler_list_item_alloc_tx_element(&sched_list_item_conf);
	uint64_t beacon_frame_time = start_time;

	if (!sched_list_item) {
		desh_error("(%s): dect_phy_api_scheduler_list_item_alloc_tx_element failed: No "
			   "memory to TX a beacon");
		return -ENOMEM;
	}
	uint16_t encoded_pdu_length = pdu_ptr - encoded_beacon_pdu;

	sched_list_item_conf->address_info.network_id = current_settings->common.network_id;
	sched_list_item_conf->address_info.transmitter_long_rd_id =
		current_settings->common.transmitter_id;
	sched_list_item_conf->address_info.receiver_long_rd_id = DECT_LONG_RD_ID_BROADCAST;

	sched_list_item_conf->cb_op_to_mdm = dect_phy_mac_cluster_beacon_to_mdm_cb;
	sched_list_item_conf->cb_op_completed = NULL;

	sched_list_item_conf->channel = params->beacon_channel;
	sched_list_item_conf->frame_time = start_time;
	beacon_frame_time = sched_list_item_conf->frame_time;

	sched_list_item_conf->start_slot = 0;

	/* Let it run in intervals in a scheduler */
	sched_list_item_conf->interval_mdm_ticks = interval_mdm_ticks;
	sched_list_item_conf->length_slots = slot_count;
	sched_list_item_conf->length_subslots = 0;

	sched_list_item_conf->tx.phy_lbt_period = 0;

	sched_list_item->priority = DECT_PRIORITY1_TX;

	sched_list_item->sched_config.tx.encoded_payload_pdu_size = encoded_pdu_length;
	memcpy(sched_list_item->sched_config.tx.encoded_payload_pdu, encoded_beacon_pdu,
	       sched_list_item->sched_config.tx.encoded_payload_pdu_size);

	memcpy(beacon_data.encoded_cluster_beacon_pdu, encoded_beacon_pdu, encoded_pdu_length);
	beacon_data.encoded_cluster_beacon_pdu_len = encoded_pdu_length;

	sched_list_item->sched_config.tx.header_type = DECT_PHY_HEADER_TYPE1;

	memcpy(&sched_list_item->sched_config.tx.phy_header.type_1, &phy_header.type_1,
	       sizeof(phy_header.type_1));

	sched_list_item->phy_op_handle = DECT_PHY_MAC_BEACON_TX_HANDLE;

	/* Add beacon tx operation to scheduler list */
	if (!dect_phy_api_scheduler_list_item_add(sched_list_item)) {
		desh_error("(%s): dect_phy_api_scheduler_list_item_add failed\n", (__func__));
		dect_phy_api_scheduler_list_item_dealloc(sched_list_item);
		return -EBUSY;
	}

	/* Schedule RACH RXes. Note: no specific LMS for all of these, ie. not strictly
	 * as mac spec intended. However, LBT shall be used and is used in desh when sending
	 * to random access resource.
	 */
	uint32_t rach_handle = DECT_PHY_MAC_BEACON_RX_RACH_HANDLE_START;
	uint64_t rach_frame_time = beacon_frame_time;

	uint64_t last_valid_rach_rx_frame_time =
		rach_frame_time + (DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS *
				   DECT_PHY_MAC_CLUSTER_BEACON_RA_VALIDITY);

	while (rach_frame_time <= last_valid_rach_rx_frame_time) {
		struct dect_phy_api_scheduler_list_item_config *rach_list_item_conf;
		struct dect_phy_api_scheduler_list_item *rach_list_item =
			dect_phy_api_scheduler_list_item_alloc_rx_element(&rach_list_item_conf);

		if (!rach_list_item) {
			break;
		}

		rach_list_item_conf->cb_op_completed = NULL;

		rach_list_item_conf->channel = params->beacon_channel;
		rach_list_item_conf->frame_time = rach_frame_time;
		rach_list_item_conf->interval_mdm_ticks = interval_mdm_ticks;

		rach_list_item_conf->start_slot = (DECT_PHY_MAC_CLUSTER_BEACON_RA_START_SUBSLOT /
						   DECT_RADIO_FRAME_SUBSLOT_COUNT_IN_SLOT);
		rach_list_item_conf->length_slots = DECT_PHY_MAC_CLUSTER_BEACON_RA_LENGTH_SLOTS;
		rach_list_item_conf->length_subslots = 0;

		rach_list_item_conf->rx.mode = NRF_MODEM_DECT_PHY_RX_MODE_CONTINUOUS;
		rach_list_item_conf->rx.expected_rssi_level =
			current_settings->rx.expected_rssi_level;
		rach_list_item_conf->rx.duration =
			0; /* length_slots used instead duration variable */
		rach_list_item_conf->rx.network_id = current_settings->common.network_id;

		/* Only receive the ones destinated to this beacon: */
		rach_list_item_conf->rx.filter.is_short_network_id_used = true;
		rach_list_item_conf->rx.filter.short_network_id =
			(uint8_t)(current_settings->common.network_id & 0xFF);
		rach_list_item_conf->rx.filter.receiver_identity =
			current_settings->common.short_rd_id;

		rach_list_item->priority = DECT_PRIORITY2_RX;
		rach_list_item->phy_op_handle = rach_handle;

		if (!dect_phy_api_scheduler_list_item_add(rach_list_item)) {
			desh_error("(%s): dect_phy_api_scheduler_list_item_add for RACH failed",
				   (__func__));
			ret = -EBUSY;
			dect_phy_api_scheduler_list_item_dealloc(rach_list_item);
			break;
		}

		rach_frame_time = rach_frame_time + (DECT_PHY_MAC_CLUSTER_BEACON_RA_REPETITION *
						     DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS);
		rach_handle++;
		if (rach_handle > DECT_PHY_MAC_BEACON_RX_RACH_HANDLE_END) {
			rach_handle = DECT_PHY_MAC_BEACON_RX_RACH_HANDLE_START;
		}
	}

	beacon_data.running = true;

	desh_print("Scheduled beacon TX: "
		   "interval %dms, tx pwr %d dbm, channel %d, payload PDU byte count: %d",
		   DECT_PHY_MAC_CLUSTER_BEACON_INTERVAL_MS, params->tx_power_dbm,
		   params->beacon_channel, encoded_pdu_length);

	return 0;
}

static void dect_phy_mac_cluster_beacon_scheduler_list_items_remove(void)
{
	dect_phy_api_scheduler_list_item_remove_dealloc_by_phy_op_handle(
		DECT_PHY_MAC_BEACON_LMS_RSSI_SCAN_HANDLE);
	dect_phy_api_scheduler_list_item_remove_dealloc_by_phy_op_handle(
		DECT_PHY_MAC_BEACON_TX_HANDLE);
	dect_phy_api_scheduler_list_item_remove_dealloc_by_phy_op_handle_range(
		DECT_PHY_MAC_BEACON_RX_RACH_HANDLE_START, DECT_PHY_MAC_BEACON_RX_RACH_HANDLE_END);
}

void dect_phy_mac_cluster_beacon_tx_stop(void)
{
	dect_phy_mac_cluster_beacon_scheduler_list_items_remove();
	beacon_data.running = false;
}

bool dect_phy_mac_cluster_beacon_is_running(void)
{
	return beacon_data.running;
}

void dect_phy_mac_cluster_beacon_update(void)
{
	uint8_t *pdu_ptr = beacon_data.encoded_cluster_beacon_pdu;
	union nrf_modem_dect_phy_hdr phy_header;

	if (!beacon_data.running) {
		return;
	}

	beacon_data.next_sfn++;

	/* Re-encode cluster beacon */
	int ret = dect_phy_mac_cluster_beacon_encode(&beacon_data.start_params, &pdu_ptr,
						     &phy_header);
	if (ret < 0) {
		desh_error("(%s): Failed to re-encode beacon", __func__);
		return;
	}

	uint16_t encoded_pdu_length = pdu_ptr - beacon_data.encoded_cluster_beacon_pdu;

	beacon_data.encoded_cluster_beacon_pdu_len = encoded_pdu_length;
	dect_phy_api_scheduler_list_item_pdu_payload_update_by_phy_handle(
		DECT_PHY_MAC_BEACON_TX_HANDLE, beacon_data.encoded_cluster_beacon_pdu,
		beacon_data.encoded_cluster_beacon_pdu_len);
}

static int dect_phy_mac_cluster_beacon_association_resp_pdu_encode(
	struct dect_phy_commmon_op_pdc_rcv_params *rcv_params,
	dect_phy_mac_common_header_t *common_header,
	dect_phy_mac_association_req_t *association_req, uint8_t **target_ptr, /* In/Out */
	union nrf_modem_dect_phy_hdr *out_phy_header)
{
	struct dect_phy_settings *current_settings = dect_common_settings_ref_get();
	struct dect_phy_header_type2_format1_t header = {
		.packet_length = 1, /* calculated later based on needs */
		.packet_length_type = DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS,
		.format = DECT_PHY_HEADER_FORMAT_001, /* No HARQ feedback requested */
		.short_network_id = (uint8_t)(current_settings->common.network_id & 0xFF),
		.transmitter_identity_hi = (uint8_t)(current_settings->common.short_rd_id >> 8),
		.transmitter_identity_lo = (uint8_t)(current_settings->common.short_rd_id & 0xFF),
		.df_mcs = rcv_params->rx_mcs,
		.transmit_power = dect_common_utils_dbm_to_phy_tx_power(rcv_params->rx_pwr_dbm),
		.receiver_identity_hi =
			(uint8_t)(rcv_params->last_received_pcc_transmitter_short_rd_id >> 8),
		.receiver_identity_lo =
			(uint8_t)(rcv_params->last_received_pcc_transmitter_short_rd_id & 0xFF),
		.feedback.format1.format = 0,
	};
	dect_phy_mac_type_header_t type_header_resp = {
		.version = 0,
		.security = 0, /* 0b00: Not used */
		.type = DECT_PHY_MAC_HEADER_TYPE_UNICAST,
	};
	dect_phy_mac_common_header_t common_header_resp = {
		.type = DECT_PHY_MAC_HEADER_TYPE_UNICAST,
		.reset = 1,
		.seq_nbr = common_header->seq_nbr,
		.nw_id = current_settings->common.network_id,
		.transmitter_id = current_settings->common.transmitter_id,
		.receiver_id = common_header->transmitter_id,
	};
	uint8_t *pdu_ptr = *target_ptr;

	pdu_ptr = dect_phy_mac_pdu_type_header_encode(&type_header_resp, pdu_ptr);
	pdu_ptr = dect_phy_mac_pdu_common_header_encode(&common_header_resp, pdu_ptr);

	sys_dlist_t sdu_list;
	dect_phy_mac_sdu_t *data_sdu_list_item =
		(dect_phy_mac_sdu_t *)k_calloc(1, sizeof(dect_phy_mac_sdu_t));
	if (data_sdu_list_item == NULL) {
		return -ENOMEM;
	}
	uint16_t payload_data_len = DECT_PHY_MAC_ASSOCIATION_RESP_MIN_LEN;
	dect_phy_mac_mux_header_t mux_header1 = {
		.mac_ext = DECT_PHY_MAC_EXT_8BIT_LEN,
		.ie_type = DECT_PHY_MAC_IE_TYPE_ASSOCIATION_RESP,
		.payload_length = payload_data_len,
	};

	/* Encode association response: we are dummy beacon and accepting everything  */
	dect_phy_mac_association_resp_t association_resp = {
		.ack_bit = 1,
		.group_bit = 0,
		.harq_conf_bit = 0, /* HARQ config accepted as in a request */
		.flow_count = 7,    /* 0b111: all flows accepted as in request */
	};

	data_sdu_list_item->mux_header = mux_header1;
	data_sdu_list_item->message_type = DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_RESP;
	data_sdu_list_item->message.association_resp = association_resp;

	sys_dlist_init(&sdu_list);
	sys_dlist_append(&sdu_list, &data_sdu_list_item->dnode);
		/* HS_DECT: In fixed scheduling mode, add an EXTENSION IE telling PT its assigned index
	 * and (optionally) the slot map.
		*/
		if (current_settings->mac_sched.mode == DECT_MAC_SCHED_FIXED) {

			/* Assign PT index at FT based on PT long rd id */
			int pt_idx = dect_phy_mac_ft_assoc_add_or_update(
				common_header->transmitter_id, /* PT long rd id */
				(uint16_t)rcv_params->last_received_pcc_transmitter_short_rd_id,
				(int8_t)rcv_params->rx_pwr_dbm,
				rcv_params->time);

			if (pt_idx < 0) {
				/* FT full: you can also choose to reject association here.
				* For now, still respond ACK but without assignment IE.
				*/
				desh_warn("FT assoc table full, cannot assign PT index (err %d)", pt_idx);
			} else {
				/* Build Extension IE payload */
				uint8_t payload[32];
				uint8_t *w = payload;

				*w++ = HS_DECT_IE_VER;   /* IE version */
				*w++ = 1;              /* mode: 1=fixed, 0=random */
				*w++ = (uint8_t)pt_idx; /* assigned PT index (1..max_pts) */
				*w++ = (uint8_t)current_settings->mac_sched.max_pts;
				*w++ = (uint8_t)current_settings->mac_sched.superframe_len; /* interpret as slots/frame */

				/* slot map (PT1..PTmax): start/end */
				for (int i = 0; i < current_settings->mac_sched.max_pts; i++) {
					*w++ = (uint8_t)current_settings->mac_sched.pt_slots[i].start_subslot;
					*w++ = (uint8_t)current_settings->mac_sched.pt_slots[i].end_subslot;
				}

				uint16_t ext_len = (uint16_t)(w - payload);

				dect_phy_mac_sdu_t *ext_sdu =
					(dect_phy_mac_sdu_t *)k_calloc(1, sizeof(dect_phy_mac_sdu_t));
				if (ext_sdu != NULL) {
					ext_sdu->mux_header.mac_ext = DECT_PHY_MAC_EXT_16BIT_LEN;
					ext_sdu->mux_header.ie_type = DECT_PHY_MAC_IE_TYPE_EXTENSION;
					ext_sdu->mux_header.ie_ext = HS_DECT_IE_EXT_TYPE_SCHED_ASSIGN;
					ext_sdu->mux_header.payload_length = ext_len;

					ext_sdu->message_type = DECT_PHY_MAC_MESSAGE_ESCAPE;
					ext_sdu->message.common_msg.data_length = ext_len;
					memcpy(ext_sdu->message.common_msg.data, payload, ext_len);

					sys_dlist_append(&sdu_list, &ext_sdu->dnode);
				}
			}
		}

	pdu_ptr = dect_phy_mac_pdu_sdus_encode(pdu_ptr, &sdu_list);

	/* Length so far  */
	uint16_t encoded_pdu_length = pdu_ptr - *target_ptr;

	header.packet_length = dect_common_utils_phy_packet_length_calculate(
		encoded_pdu_length, header.packet_length_type, header.df_mcs);
	if (header.packet_length < 0) {
		printk("(%s): Phy pkt len calculation failed\n", (__func__));
		return -EINVAL;
	}
	int16_t total_byte_count =
		dect_common_utils_slots_in_bytes(header.packet_length, header.df_mcs);

	if (total_byte_count <= 0) {
		printk("Unsupported slot/mcs combination\n");
		return -EINVAL;
	}
	/* Fill padding if needed */
	int16_t padding_need = total_byte_count - encoded_pdu_length;
	int err = dect_phy_mac_pdu_sdu_list_add_padding(&pdu_ptr, &sdu_list, padding_need);

	if (err) {
		desh_warn("(%s): Failed to add padding: err %d (continue)", __func__, err);
	}

	*target_ptr = pdu_ptr;

	union nrf_modem_dect_phy_hdr phy_header;

	memcpy(out_phy_header, &header, sizeof(phy_header.type_2));
	hs_sdu_list_free_all(&sdu_list);
	return header.packet_length;
}

void dect_phy_mac_cluster_beacon_association_req_handle(
	struct dect_phy_commmon_op_pdc_rcv_params *rcv_params,
	dect_phy_mac_common_header_t *common_header,
	dect_phy_mac_association_req_t *association_req)
{
	if (!beacon_data.running) {
		printk("Beacon not running and received Association Req from %u (0x%04x)\n",
		       common_header->transmitter_id, common_header->transmitter_id);
		return;
	}

	union nrf_modem_dect_phy_hdr phy_header;
	int ret;
	uint8_t slot_count = 0;
	uint8_t encoded_data_to_send2[DECT_DATA_MAX_LEN];
	uint8_t *pdu_ptr = encoded_data_to_send2;

	memset(encoded_data_to_send2, 0, DECT_DATA_MAX_LEN);

	/* Encode response PDU to be sent */
	ret = dect_phy_mac_cluster_beacon_association_resp_pdu_encode(
		rcv_params, common_header, association_req, &pdu_ptr, &phy_header);
	if (ret < 0) {
		return;
	}
	slot_count = ret + 1;
	uint16_t encoded_pdu_length = pdu_ptr - encoded_data_to_send2;

	/* Schedule association response*/
	uint64_t req_received = rcv_params->time;
	uint64_t req_len = (rcv_params->last_received_pcc_phy_len_type ==
			    DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS)
				   ? (rcv_params->last_received_pcc_phy_len *
				      DECT_RADIO_SLOT_DURATION_IN_MODEM_TICKS)
				   : (rcv_params->last_received_pcc_phy_len *
				      DECT_RADIO_SUBSLOT_DURATION_IN_MODEM_TICKS);

	/* Note: we are not sending response right after 0.5 frames as in DECT-2020
	 * when dect_delay set. We are adding request len more time for scheduling. That
	 * is in specs as MAC spec says: "When RD transmits Random Access resources withs
	 * Physical Layer Control Field: Type 2, Header Format: 001 and expects MAC PDU as
	 * a response, the RD should consider the response window as a length of one frame.
	 */
	uint64_t resp_start_time =
		req_received + req_len + (DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS / 2);

	struct nrf_modem_dect_phy_tx_params tx_op; /* We need toi bypass scheduler */
	struct dect_phy_settings *current_settings = dect_common_settings_ref_get();

	tx_op.bs_cqi = NRF_MODEM_DECT_PHY_BS_CQI_NOT_USED;
	tx_op.carrier = beacon_data.start_params.beacon_channel;
	tx_op.data_size = encoded_pdu_length;
	tx_op.data = encoded_data_to_send2;
	tx_op.lbt_period = NRF_MODEM_DECT_LBT_PERIOD_MIN;
	tx_op.lbt_rssi_threshold_max = current_settings->rssi_scan.busy_threshold;
	tx_op.network_id = current_settings->common.network_id;
	tx_op.phy_header = &phy_header;
	tx_op.phy_type = DECT_PHY_HEADER_TYPE2;
	tx_op.handle = DECT_PHY_MAC_BEACON_RA_RESP_TX_HANDLE;
	tx_op.start_time = resp_start_time;
	ret = nrf_modem_dect_phy_tx(&tx_op);
	if (ret) {
		printk("(%s): nrf_modem_dect_phy_tx failed %d (handle %d)\n", (__func__), ret,
		       tx_op.handle);
	}
}
/* In case of association request with ack_bit=0 or other need to reject association, we can send an association response with ack_bit=0. */

void dect_phy_mac_cluster_beacon_association_reject_send(
	struct dect_phy_commmon_op_pdc_rcv_params *rcv_params,
	dect_phy_mac_common_header_t *common_header)
{
	if (!beacon_data.running) {
		return;
	}

	struct dect_phy_settings *current_settings = dect_common_settings_ref_get();

	/* Build a minimal Association Response with ack_bit=0 */
	union nrf_modem_dect_phy_hdr phy_header;
	uint8_t encoded[DECT_DATA_MAX_LEN];
	uint8_t *pdu_ptr = encoded;

	memset(encoded, 0, sizeof(encoded));

	/* PHY header same style as normal association resp */
	struct dect_phy_header_type2_format1_t header = {
		.packet_length = 1,
		.packet_length_type = DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS,
		.format = DECT_PHY_HEADER_FORMAT_001,
		.short_network_id = (uint8_t)(current_settings->common.network_id & 0xFF),
		.transmitter_identity_hi = (uint8_t)(current_settings->common.short_rd_id >> 8),
		.transmitter_identity_lo = (uint8_t)(current_settings->common.short_rd_id & 0xFF),
		.df_mcs = rcv_params->rx_mcs,
		.transmit_power = dect_common_utils_dbm_to_phy_tx_power(rcv_params->rx_pwr_dbm),
		.receiver_identity_hi =
			(uint8_t)(rcv_params->last_received_pcc_transmitter_short_rd_id >> 8),
		.receiver_identity_lo =
			(uint8_t)(rcv_params->last_received_pcc_transmitter_short_rd_id & 0xFF),
		.feedback.format1.format = 0,
	};

	dect_phy_mac_type_header_t type_header_resp = {
		.version = 0,
		.security = 0,
		.type = DECT_PHY_MAC_HEADER_TYPE_UNICAST,
	};

	dect_phy_mac_common_header_t common_header_resp = {
		.type = DECT_PHY_MAC_HEADER_TYPE_UNICAST,
		.reset = 1,
		.seq_nbr = common_header->seq_nbr,
		.nw_id = current_settings->common.network_id,
		.transmitter_id = current_settings->common.transmitter_id,
		.receiver_id = common_header->transmitter_id,
	};

	pdu_ptr = dect_phy_mac_pdu_type_header_encode(&type_header_resp, pdu_ptr);
	pdu_ptr = dect_phy_mac_pdu_common_header_encode(&common_header_resp, pdu_ptr);

	sys_dlist_t sdu_list;
	sys_dlist_init(&sdu_list);

	dect_phy_mac_sdu_t *resp_sdu =
		(dect_phy_mac_sdu_t *)k_calloc(1, sizeof(dect_phy_mac_sdu_t));
	if (resp_sdu == NULL) {
		return;
	}

	resp_sdu->mux_header.mac_ext = DECT_PHY_MAC_EXT_8BIT_LEN;
	resp_sdu->mux_header.ie_type = DECT_PHY_MAC_IE_TYPE_ASSOCIATION_RESP;
	resp_sdu->mux_header.payload_length = DECT_PHY_MAC_ASSOCIATION_RESP_MIN_LEN;

	/* ACK=0 => reject */
	resp_sdu->message_type = DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_RESP;
	resp_sdu->message.association_resp.ack_bit = 0;
	resp_sdu->message.association_resp.group_bit = 0;
	resp_sdu->message.association_resp.harq_conf_bit = 0;
	resp_sdu->message.association_resp.flow_count = 0;

	sys_dlist_append(&sdu_list, &resp_sdu->dnode);

	pdu_ptr = dect_phy_mac_pdu_sdus_encode(pdu_ptr, &sdu_list);

	uint16_t encoded_len = pdu_ptr - encoded;

	header.packet_length = dect_common_utils_phy_packet_length_calculate(
		encoded_len, header.packet_length_type, header.df_mcs);
	if ((int)header.packet_length < 0) {
		return;
	}

	memcpy(&phy_header.type_2, &header, sizeof(phy_header.type_2));

	/* Schedule response similarly to normal association response */
	uint64_t req_received = rcv_params->time;
	uint64_t req_len = (rcv_params->last_received_pcc_phy_len_type ==
			    DECT_PHY_HEADER_PKT_LENGTH_TYPE_SLOTS)
				   ? (rcv_params->last_received_pcc_phy_len *
				      DECT_RADIO_SLOT_DURATION_IN_MODEM_TICKS)
				   : (rcv_params->last_received_pcc_phy_len *
				      DECT_RADIO_SUBSLOT_DURATION_IN_MODEM_TICKS);

	uint64_t resp_start_time =
		req_received + req_len + (DECT_RADIO_FRAME_DURATION_IN_MODEM_TICKS / 2);

	struct nrf_modem_dect_phy_tx_params tx_op;

	tx_op.bs_cqi = NRF_MODEM_DECT_PHY_BS_CQI_NOT_USED;
	tx_op.carrier = beacon_data.start_params.beacon_channel;
	tx_op.data_size = encoded_len;
	tx_op.data = encoded;
	tx_op.lbt_period = NRF_MODEM_DECT_LBT_PERIOD_MIN;
	tx_op.lbt_rssi_threshold_max = current_settings->rssi_scan.busy_threshold;
	tx_op.network_id = current_settings->common.network_id;
	tx_op.phy_header = &phy_header;
	tx_op.phy_type = DECT_PHY_HEADER_TYPE2;
	tx_op.handle = DECT_PHY_MAC_BEACON_RA_RESP_TX_HANDLE;
	tx_op.start_time = resp_start_time;

	(void)nrf_modem_dect_phy_tx(&tx_op);
}


void dect_phy_mac_cluster_beacon_status_print(void)
{
	desh_print("Cluster beacon status:");
	desh_print("  Beacon running: %s", beacon_data.running ? "yes" : "no");
	if (beacon_data.running) {
		desh_print("  Beacon channel:                %d",
			   beacon_data.start_params.beacon_channel);
		desh_print("  Beacon tx power:               %d dBm",
			   beacon_data.start_params.tx_power_dbm);
		desh_print("  Beacon interval:               %d ms",
			   DECT_PHY_MAC_CLUSTER_BEACON_INTERVAL_MS);
		desh_print("  Beacon payload PDU byte count: %d",
			   beacon_data.encoded_cluster_beacon_pdu_len);
	}
}

int64_t dect_phy_mac_cluster_beacon_rcv_time_shift_calculate(
	dect_phy_mac_cluster_beacon_period_t interval,
	uint64_t last_rcv_time,
	uint64_t now_rcv_time)
{
	__ASSERT_NO_MSG(last_rcv_time < now_rcv_time);

	uint64_t next_beacon_frame_start, beacon_interval_mdm_ticks, prev_frame_start;
	uint64_t now_diff_to_prev_frame;
	uint64_t now_diff_to_next_frame;
	int64_t diff_out = 0;
	int32_t beacon_interval_ms = dect_phy_mac_pdu_cluster_beacon_period_in_ms(interval);

	beacon_interval_mdm_ticks = MS_TO_MODEM_TICKS(beacon_interval_ms);

	next_beacon_frame_start = last_rcv_time;
	while (next_beacon_frame_start < now_rcv_time) {
		next_beacon_frame_start += beacon_interval_mdm_ticks;
	}
	prev_frame_start = next_beacon_frame_start - beacon_interval_mdm_ticks;

	now_diff_to_prev_frame = now_rcv_time - prev_frame_start;

	now_diff_to_next_frame = next_beacon_frame_start - now_rcv_time;

	if (now_diff_to_prev_frame < now_diff_to_next_frame) {
		/* we have received later than expected */
		diff_out = now_diff_to_prev_frame;
	} else {
		/* we have received earlier than expected */
		diff_out = -now_diff_to_next_frame;
	}
	return diff_out;
}
