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
#include <nrf_modem_dect_phy.h>

#include "desh_print.h"
#include "dect_common.h"
#include "dect_phy_common.h"
#include "dect_common_utils.h"
#include "dect_common_pdu.h"
#include "dect_phy_ctrl.h"
#include "dect_common_settings.h"
#include "dect_phy_mac_pdu.h"
#include "dect_phy_mac_nbr.h"
#include "dect_phy_mac_cluster_beacon.h"
#include "dect_phy_mac_client.h"
#include "dect_phy_mac.h"
#include "dect_phy_mac_ft_assoc.h"

/**************************************************************************************************/

static void dect_phy_mac_message_print(dect_phy_mac_message_type_t message_type,
				       dect_phy_mac_message_t *message)
{
	switch (message_type) {
	case DECT_PHY_MAC_MESSAGE_TYPE_DATA_SDU: {
		unsigned char ascii_data[DECT_DATA_MAX_LEN];

		memcpy(ascii_data, message->data_sdu.data, message->data_sdu.data_length);
		ascii_data[message->data_sdu.data_length] = '\0';

		desh_print("        DLC IE type: %s (0x%02x)",
			   dect_phy_mac_dlc_pdu_ie_type_string_get(message->data_sdu.dlc_ie_type),
			   message->data_sdu.dlc_ie_type);
		desh_print("        Received data, len %d, payload as ascii string print:\n"
			   "          %s",
			   message->data_sdu.data_length, ascii_data);
		break;
	}

	case DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_REQ: {
		desh_print("      Received Association Request message:");
		desh_print("        Setup cause:      %s (0x%02x)",
			   dect_phy_mac_pdu_association_req_setup_cause_string_get(
				   message->association_req.setup_cause),
			   message->association_req.setup_cause);
		desh_print("        Flow count:       %d", message->association_req.flow_count);
		desh_print("        Flow id[0]:       %d (0x%02x)",
			   message->association_req.flow_id, message->association_req.flow_id);
		if (message->association_req.pwr_const_bit) {
			desh_print("        Power const:      has power constraints.");
		} else {
			desh_print("        Power const:      no power constraints.");
		}
		if (message->association_req.ft_mode_bit) {
			desh_print("        FT mode:          The RD operates also in FT mode.");
			desh_print("          Network Beacon period: %d ms",
				   dect_phy_mac_pdu_nw_beacon_period_in_ms(
					   message->association_req.nw_beacon_period));
			desh_print("          Cluster Beacon period: %d ms",
				   dect_phy_mac_pdu_cluster_beacon_period_in_ms(
					   message->association_req.cluster_beacon_period));
			desh_print("          Next cluster channel:  %d",
				   message->association_req.next_cluster_channel);
			desh_print("          Time to next next:     %d microseconds",
				   message->association_req.time_to_next);
		} else {
			desh_print("        FT mode:          The RD operates only in PT Mode.");
		}
		if (message->association_req.current_cluster_channel_bit) {
			desh_print("        Current cluster channel: %d",
				   message->association_req.current_cluster_channel);
		}
		desh_print("        HARQ Process TX:  %d (0x%02x)",
			   message->association_req.harq_tx_process_count,
			   message->association_req.harq_tx_process_count);
		desh_print("        MAX HARQ RE-TX:   %d (0x%02x)",
			   message->association_req.max_harq_tx_retransmission_delay,
			   message->association_req.max_harq_tx_retransmission_delay);

		desh_print("        HARQ Process RX:  %d (0x%02x)",
			   message->association_req.harq_rx_process_count,
			   message->association_req.harq_rx_process_count);
		desh_print("        MAX HARQ RE-RX:   %d (0x%02x)",
			   message->association_req.max_harq_rx_retransmission_delay,
			   message->association_req.max_harq_rx_retransmission_delay);
		break;
	}
	case DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_RESP: {
		desh_print("      Received Association Response message:");
		if (message->association_resp.ack_bit) {
			desh_print("        Acknowledgment:  ACK");
			if (message->association_resp.flow_count == 7) {
				/* 0b111 */
				desh_print(
					"        Flow count: 0b111: "
					"All flows accepted as configured in association request.");
			} else {
				desh_print("        Flow count: %d",
					   message->association_resp.flow_count);
			}
		} else {
			desh_print("        Acknowledgment:  NACK");
			desh_print("        NACK cause:       %d",
				   message->association_resp.reject_cause);
		}
		if (message->association_resp.harq_conf_bit) {
			desh_print("        HARQ Process TX:  %d (0x%02x)",
				   message->association_resp.harq_tx_process_count,
				   message->association_resp.harq_tx_process_count);
			desh_print("        MAX HARQ RE-TX:   %d (0x%02x)",
				   message->association_resp.max_harq_tx_retransmission_delay,
				   message->association_resp.max_harq_tx_retransmission_delay);

			desh_print("        HARQ Process RX:  %d (0x%02x)",
				   message->association_resp.harq_rx_process_count,
				   message->association_resp.harq_rx_process_count);
			desh_print("        MAX HARQ RE-RX:   %d (0x%02x)",
				   message->association_resp.max_harq_rx_retransmission_delay,
				   message->association_resp.max_harq_rx_retransmission_delay);
		}
		break;
	}
	case DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_REL: {
		desh_print("      Received Association Release message:");
		desh_print("        Release Cause:  %s (value: %d)",
			   dect_phy_mac_pdu_association_rel_cause_string_get(
				   message->association_rel.rel_cause),
			   message->association_rel.rel_cause);
		break;
	}
	case DECT_PHY_MAC_MESSAGE_TYPE_CLUSTER_BEACON: {
		int beacon_interval_ms = dect_phy_mac_pdu_cluster_beacon_period_in_ms(
			message->cluster_beacon.cluster_beacon_period);
		uint64_t beacon_interval_mdm_ticks = MS_TO_MODEM_TICKS(beacon_interval_ms);

		desh_print("      Received cluster beacon:");
		desh_print("        System frame number:  %d",
			   message->cluster_beacon.system_frame_number);
		if (message->cluster_beacon.tx_pwr_bit) {
			desh_print("        Max TX power:         %d dBm",
				   dect_common_utils_phy_tx_power_to_dbm(
					   message->cluster_beacon.max_phy_tx_power));
		} else {
			desh_print("        Max PHY TX power:     not included in the beacon");
		}

		if (message->cluster_beacon.pwr_const_bit) {
			desh_print("        Power const:          The RD operating in FT mode has "
				   "power constraints.");
		} else {
			desh_print("        Power const:          The RD operating in FT mode does "
				   "not have power constraints.");
		}
		if (message->cluster_beacon.frame_offset_bit) {
			desh_print("        Frame offset:         %d subslots",
				   message->cluster_beacon.frame_offset);
		} else {
			desh_print("        Frame offset:         not included in the beacon");
		}
		if (message->cluster_beacon.next_channel_bit) {
			desh_print("        Next cluster channel: %d (different than current "
				   "cluster channel)",
				   message->cluster_beacon.next_cluster_channel);
		} else {
			desh_print("        Next cluster channel: current cluster channel.");
		}
		if (message->cluster_beacon.time_to_next_next) {
			desh_print("        Time to next next:    %d microseconds",
				   message->cluster_beacon.time_to_next);
		} else {
			desh_print("        Time to next next:    not included in the beacon.\n"
				   "                              The next cluster beacon is\n"
				   "                              transmitted based on Cluster "
				   "beacon period.");
		}
		desh_print("        Network Beacon period %d ms",
			   dect_phy_mac_pdu_nw_beacon_period_in_ms(
				   message->cluster_beacon.nw_beacon_period));
		desh_print("        Cluster Beacon period %d ms (%lld mdm ticks)",
			   beacon_interval_ms, beacon_interval_mdm_ticks);
		desh_print("        Count to trigger:     %d (coded value)",
			   message->cluster_beacon.count_to_trigger);
		desh_print("        Relative quality:     %d (coded value)",
			   message->cluster_beacon.relative_quality);
		desh_print("        Min quality:          %d (coded value)",
			   message->cluster_beacon.min_quality);
		break;
	}
	case DECT_PHY_MAC_MESSAGE_RANDOM_ACCESS_RESOURCE_IE: {
		char tmp_str[128] = {0};

		desh_print("      Received RACH IE data:");
		desh_print(
			"        Repeat:               %s",
			dect_phy_mac_pdu_cluster_beacon_repeat_string_get(message->rach_ie.repeat));
		if (message->rach_ie.repeat != DECT_PHY_MAC_RA_REPEAT_TYPE_SINGLE) {
			desh_print("          Repetition:         %d", message->rach_ie.repetition);
			desh_print("          Validity:           %d", message->rach_ie.validity);
		}
		if (message->rach_ie.sfn_included) {
			desh_print("        System frame number:   %d - resource allocation valid "
				   "from this SFN onwards",
				   message->rach_ie.system_frame_number);
		} else {
			desh_print("        System frame number:  Not included - resource "
				   "allocation immediately valid",
				   message->rach_ie.system_frame_number);
		}
		if (message->rach_ie.channel_included) {
			desh_print("        RA Channel:           %d - resource allocation is "
				   "valid in indicated channel",
				   message->rach_ie.channel1);
		} else {
			desh_print("        RA Channel:           Not included - resource "
				   "allocation is valid for current channel");
		}
		if (message->rach_ie.channel2_included) {
			desh_print("        RA Response Channel:  %d - response is sent in "
				   "indicated channel",
				   message->rach_ie.channel2);
		} else {
			desh_print("        RA Response Channel:  Not included - response is sent "
				   "in same channel as this IE");
		}
		desh_print("        Start subslot:        %d", message->rach_ie.start_subslot);
		desh_print("        Length type:          %s",
			   dect_common_utils_packet_length_type_to_string(
				   message->rach_ie.length_type, tmp_str));
		desh_print("        Length:               %d", message->rach_ie.length);

		desh_print("        Max RACH length type: %s",
			   dect_common_utils_packet_length_type_to_string(
				   message->rach_ie.max_rach_length_type, tmp_str));
		desh_print("        Max RACH length:      %d", message->rach_ie.max_rach_length);
		desh_print("        CW min sig:           %d", message->rach_ie.cw_min_sig);
		desh_print(
			"        DECT delay:           %s",
			(!message->rach_ie.dect_delay)
				? "resp win starts 3 subslots after the last subslot of the RA TX"
				: "resp win starts 0.5 frames after the start of the RA TX");
		desh_print("        Response win:         %d subslots",
			   (message->rach_ie.response_win + 1));
		desh_print("        CW max sig:           %d", message->rach_ie.cw_max_sig);
		break;
	}
	case DECT_PHY_MAC_MESSAGE_PADDING: {
		desh_print("      Received padding data, len %d, payload is not printed",
			   message->common_msg.data_length);
		break;
	}
	case DECT_PHY_MAC_MESSAGE_ESCAPE: {
		unsigned char ascii_data[DECT_DATA_MAX_LEN];

		memcpy(ascii_data, message->common_msg.data, message->common_msg.data_length);
		ascii_data[message->common_msg.data_length] = '\0';

		desh_print("      Received data, len %d, payload as ascii string print:\n"
			   "        %s",
			   message->common_msg.data_length, ascii_data);
		break;
	}
	case DECT_PHY_MAC_MESSAGE_TYPE_NONE: {
		unsigned char hex_data[DECT_DATA_MAX_LEN];
		int i;

		for (i = 0; i < DECT_DATA_MAX_LEN && i < message->common_msg.data_length; i++) {
			sprintf(&hex_data[i], "%02x ", message->common_msg.data[i]);
		}
		hex_data[i + 1] = '\0';
		desh_print("      Received SDU data, len %d, payload hex data: %s\n",
			   message->common_msg.data_length, hex_data);
		break;
	}

	default:
		break;
	}
}

static void dect_phy_mac_type_header_print(dect_phy_mac_type_header_t *type_header)
{
	char tmp_str[128] = {0};

	desh_print(" DECT NR+ MAC PDU:");
	desh_print("  MAC header:");
	desh_print("    Version: %d", type_header->version);
	desh_print("    Security: %s",
		   dect_phy_mac_pdu_security_to_string(type_header->version, tmp_str));
	desh_print("    Type: %s",
		   dect_phy_mac_pdu_header_type_to_string(type_header->type, tmp_str));
}

static void dect_phy_mac_common_header_print(dect_phy_mac_type_header_t *type_header,
					     dect_phy_mac_common_header_t *common_header)
{
	if (type_header->type == DECT_PHY_MAC_HEADER_TYPE_BEACON) {
		desh_print("      Network ID (24bit MSB):  %u (0x%06x)", common_header->nw_id,
			   common_header->nw_id);
		desh_print("      Transmitter ID:          %u (0x%08x)",
			   common_header->transmitter_id, common_header->transmitter_id);
	} else {
		desh_print("      Reset: %s", (common_header->reset > 0) ? "yes" : "no");
		desh_print("      Seq Nbr: %u", common_header->seq_nbr);

		if (type_header->type == DECT_PHY_MAC_HEADER_TYPE_UNICAST) {
			desh_print("      Receiver: %u (0x%08x)", common_header->receiver_id,
				   common_header->receiver_id);
			desh_print("      Transmitter: %u (0x%08x)", common_header->transmitter_id,
				   common_header->transmitter_id);
		} else if (type_header->type == DECT_PHY_MAC_HEADER_TYPE_BROADCAST) {
			desh_print("      Transmitter: %u (0x%08x)", common_header->transmitter_id,
				   common_header->transmitter_id);
		}
	}
}

static void dect_phy_mac_mux_header_print(const struct shell *shell,
                                          const dect_phy_mac_mux_header_t *mux_header)
{
    ARG_UNUSED(shell);
    ARG_UNUSED(mux_header);
    /* NOTE: Do not print from RX/PHY callback context. */
}

static void dect_phy_mac_sdu_print(const struct shell *shell,
                                   const dect_phy_mac_sdu_t *sdu_list_item,
                                   int sdu_nbr)
{
    ARG_UNUSED(shell);
    ARG_UNUSED(sdu_list_item);
    ARG_UNUSED(sdu_nbr);
    /* NOTE: Do not print from RX/PHY callback context. */
}


bool dect_phy_mac_handle(struct dect_phy_commmon_op_pdc_rcv_params *rcv_params)
{
    dect_phy_mac_type_header_t type_header;
    dect_phy_mac_common_header_t common_header;
    bool handled = false;
    bool print = false;

    /* ---------- HARD GUARDS ---------- */
    if (rcv_params == NULL ) {
        return false;
    }

    uint8_t *data_ptr = rcv_params->data;
    uint32_t data_len = rcv_params->data_length;

    /* Need at least 1 byte for type header decode */
    if (data_len < 1U) {
        return false;
    }
    /* ------------------------------- */

    handled = dect_phy_mac_pdu_type_header_decode(data_ptr, data_len, &type_header);
    if (!handled) {
        return false;
    }

    /* Print only if:
     * - beacon tx ongoing and we have announced RA resources.
     * - client is associating/associated with the target device and the received message is not a beacon from target
     * - beacon scan / rx cmd is ongoing
     *
     * IMPORTANT: do not actually print here (RX context). Keep the flag for passing to safe components.
     */
    if (dect_phy_mac_cluster_beacon_is_running()) {
        print = true;
    }

    if (dect_phy_mac_client_associated_by_target_short_rd_id(
            rcv_params->last_received_pcc_transmitter_short_rd_id)) {
        print = true;
        if (type_header.type == DECT_PHY_MAC_HEADER_TYPE_BEACON) {
            print = false;
        }
    }

    if (dect_phy_ctrl_rx_is_ongoing()) {
        print = true;
    }

    /* Need at least 2 bytes because common header decode uses data_ptr+1 and data_len-1 */
    if (data_len < 2U) {
        return false;
    }

    const uint8_t *p_ptr = data_ptr + 1U;

    /* Decode MAC Common header */
    handled = dect_phy_mac_pdu_common_header_decode(p_ptr, data_len - 1U, &type_header, &common_header);
    if (!handled) {
        return false;
    }

    /* Compute header length and validate against buffer */
    uint8_t header_len = dect_phy_mac_pdy_type_n_common_header_len_get(type_header.type);
    if (header_len == 0U || header_len > data_len) {
        return false;
    }

    uint8_t *payload_ptr = data_ptr + header_len;
    uint32_t payload_len = data_len - header_len;

    /* Decode SDUs only if payload exists */
    sys_dlist_t sdu_list;
    sys_dlist_init(&sdu_list);

    if (payload_len == 0U) {
        return true;
    }

    handled = dect_phy_mac_pdu_sdus_decode(payload_ptr, payload_len, &sdu_list);
    if (!handled) {
        /* Do not desh_error() here (RX context) */
        goto out_free;
    }

    /* Extract what we need from SDUs */
    dect_phy_mac_cluster_beacon_t *beacon_msg = NULL;
    dect_phy_mac_random_access_resource_ie_t *ra_ie = NULL;
    dect_phy_mac_association_resp_t *association_resp = NULL;

    dect_phy_mac_sdu_t *sdu_list_item;
    uint32_t sdu_count = 0;

    SYS_DLIST_FOR_EACH_CONTAINER(&sdu_list, sdu_list_item, dnode) {

        /* Do not print from RX context. Keep 'print' flag for other functions if needed. */
        if (print) {
            /* If you later move printing to a worker, you can capture SDU data here. */
            (void)sdu_count;
        }

        switch (sdu_list_item->message_type) {

        case DECT_PHY_MAC_MESSAGE_TYPE_CLUSTER_BEACON:
            beacon_msg = &sdu_list_item->message.cluster_beacon;
            break;

        case DECT_PHY_MAC_MESSAGE_RANDOM_ACCESS_RESOURCE_IE:
            /* There could be many of these; store only the last */
            ra_ie = &sdu_list_item->message.rach_ie;
            break;

        case DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_RESP:
            association_resp = &sdu_list_item->message.association_resp;
            break;

        case DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_REL: {
            /* FT: PT dissociated -> remove from FT assoc table (NO PRINT here) */
            uint32_t pt_long_rd_id = common_header.transmitter_id;
            (void)dect_phy_mac_ft_assoc_remove(pt_long_rd_id);
            break;
        }

        default:
            break;
        }

        sdu_count++;
    }

    /* Neighbor store (safe: ensure this function does not print in RX context) */
    if (beacon_msg != NULL && ra_ie != NULL) {
        dect_phy_mac_nbr_info_store_n_update(
            &rcv_params->time,
            rcv_params->rx_channel,
            common_header.nw_id,
            rcv_params->last_received_pcc_short_nw_id,
            common_header.transmitter_id,
            rcv_params->last_received_pcc_transmitter_short_rd_id,
            beacon_msg,
            ra_ie,
            print
        );
    }

    if (association_resp != NULL) {
        dect_phy_mac_client_associate_resp_handle(&common_header, association_resp);
    }

out_free:
    /* Remove all nodes from list and dealloc */
    while (!sys_dlist_is_empty(&sdu_list)) {
        sys_dnode_t *node = sys_dlist_get(&sdu_list);
        if (node != NULL) {
            dect_phy_mac_sdu_t *item = CONTAINER_OF(node, dect_phy_mac_sdu_t, dnode);
            k_free(item);
        }
    }

    return handled;
}

/**************************************************************************************************/

bool dect_phy_mac_direct_pdc_handle(struct dect_phy_commmon_op_pdc_rcv_params *rcv_params)
{
    dect_phy_mac_type_header_t type_header;
    dect_phy_mac_common_header_t common_header;
    bool handled = false;

    /* ---------- HARD GUARDS: prevent out-of-bounds / NULL ---------- */
    if (rcv_params == NULL) {
        return false;
    }

    uint8_t *data_ptr = rcv_params->data;
    uint32_t data_len = rcv_params->data_length;

    /* We use data_ptr+1 and (data_len-1) when decoding header */
    if (data_len < 2U) {
        return false;
    }
    /* ---------------------------------------------------------------- */

    /* Common header decode uses bytes after the first octet */
    const uint8_t *p_ptr = data_ptr + 1;

    handled = dect_phy_mac_pdu_common_header_decode(p_ptr, data_len - 1U, &type_header, &common_header);
    if (!handled) {
        return false;
    }
			/* HS_DECT: FT assoc-table update (best-effort)
		* We add/update the PT on ANY successfully decoded common header.
		* This is needed because some RA/associate traffic may not decode into
		* MESSAGE_TYPE_ASSOCIATION_REQ in this direct handler.
		*/
		{
			struct dect_phy_settings *s = dect_common_settings_ref_get();
			if (s != NULL && s->mac_sched.role == DECT_MAC_ROLE_FT) {
				int16_t rssi_level = rcv_params->rx_status.rssi_2 / 2;

				(void)dect_phy_mac_ft_assoc_add_or_update(
					common_header.transmitter_id,
					rcv_params->last_received_pcc_transmitter_short_rd_id,
					(int8_t)rssi_level,
					rcv_params->time
				);
			}
		}

    /* Determine header length now that type_header.type is valid */
    uint8_t header_len = dect_phy_mac_pdy_type_n_common_header_len_get(type_header.type);

    /* header_len must be within the received buffer */
    if (header_len == 0U || header_len > data_len) {
        return false;
    }

    uint8_t *payload_ptr = data_ptr + header_len;
    uint32_t payload_len = data_len - header_len;

    /* Nothing to decode beyond header */
    if (payload_len == 0U) {
        return true;
    }

    sys_dlist_t sdu_list;
    sys_dlist_init(&sdu_list);

    handled = dect_phy_mac_pdu_sdus_decode(payload_ptr, payload_len, &sdu_list);

    if (handled) {
        bool pt_says_fixed = false;
        bool got_hs_policy = false;

        dect_phy_mac_sdu_t *sdu_list_item;

        SYS_DLIST_FOR_EACH_CONTAINER(&sdu_list, sdu_list_item, dnode) {

            /* HS_DECT: parse vendor extension IE for PT policy */
            if (sdu_list_item->mux_header.ie_type == DECT_PHY_MAC_IE_TYPE_EXTENSION &&
                sdu_list_item->mux_header.ie_ext == HS_DECT_IE_EXT_TYPE_ASSOC_POLICY &&
                sdu_list_item->message.common_msg.data_length >= 2) {

                const uint8_t *p = sdu_list_item->message.common_msg.data;
                if (p != NULL && p[0] == HS_DECT_ASSOC_EXT_VER) {
                    got_hs_policy = true;
                    pt_says_fixed = ((p[1] & HS_DECT_ASSOC_FLAG_PT_FIXED_MODE) != 0);
                }
            }

            /* Handle association request */
            if (sdu_list_item->message_type == DECT_PHY_MAC_MESSAGE_TYPE_ASSOCIATION_REQ) {

                struct dect_phy_settings *s = dect_common_settings_ref_get();
                if (s == NULL) {
                    continue;
                }

                /* If FT is fixed, PT must advertise fixed 
                if (s->mac_sched.mode == DECT_MAC_SCHED_FIXED) {
                    if (!got_hs_policy || !pt_says_fixed) {
                        dect_phy_mac_cluster_beacon_association_reject_send(rcv_params, &common_header);
                        continue;
                    }
                }
				
                FT: store PT association (ISR-safe; no prints here) 
                if (s->mac_sched.role == DECT_MAC_ROLE_FT) {
                    int16_t rssi_level = rcv_params->rx_status.rssi_2 / 2;

                    (void)dect_phy_mac_ft_assoc_add_or_update(
                        common_header.transmitter_id,
                        rcv_params->last_received_pcc_transmitter_short_rd_id,
                        (int8_t)rssi_level,
                        rcv_params->time
                    );
                }
				*/
                /* Continue normal association flow */
                dect_phy_mac_cluster_beacon_association_req_handle(
                    rcv_params,
                    &common_header,
                    &sdu_list_item->message.association_req
                );
            }
        }
    }

    /* Free SDU list nodes allocated by decode */
    while (!sys_dlist_is_empty(&sdu_list)) {
        sys_dnode_t *node = sys_dlist_get(&sdu_list);
        if (node != NULL) {
            dect_phy_mac_sdu_t *item = CONTAINER_OF(node, dect_phy_mac_sdu_t, dnode);
            k_free(item);
        }
    }

    return handled;
}

