/**
 * @file rdm_handle_data.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "monitor.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_device_info.h"
#include "rdm_e120.h"
#include "rdm_handlers.h"
#include "rdm_sub_devices.h"
#include "rdm_send.h"

static bool rdm_muted = false;	///<

/**
 * @ingroup rdm_handlers
 *
 * @return
 */
const bool rdm_is_muted(void) {
	return rdm_muted;
}

/**
 * @ingroup rdm_handlers
 *
 * Main function for handling RDM data.
 */
void rdm_handle_data(uint8_t *rdm_data) {
	bool rdm_packet_is_for_me = false;
	bool rdm_packet_is_broadcast = false;
	bool rdm_packet_is_vendorcast = false;

	struct _rdm_command *rdm_cmd = (struct _rdm_command *) rdm_data;

	const uint8_t command_class = rdm_cmd->command_class;
	const uint16_t param_id = (rdm_cmd->param_id[0] << 8) + rdm_cmd->param_id[1];
	const uint8_t *uid_device = rdm_device_info_get_uuid();

	monitor_line(MONITOR_LINE_RDM_CC, "Command class [%.2X]:%d, param_id [%.2x%.2x]:%d", command_class, command_class, rdm_cmd->param_id[0], rdm_cmd->param_id[1], param_id);

	if (memcmp(rdm_cmd->destination_uid, UID_ALL, RDM_UID_SIZE) == 0) {
		rdm_packet_is_broadcast = true;
	}

	if ((memcmp(rdm_cmd->destination_uid, uid_device, 2) == 0) && (memcmp(&rdm_cmd->destination_uid[2], UID_ALL, 4) == 0)) {
		rdm_packet_is_vendorcast = true;
		rdm_packet_is_for_me = true;
	} else if (memcmp(rdm_cmd->destination_uid, uid_device, RDM_UID_SIZE) == 0) {
		rdm_packet_is_for_me = true;
	}

	if ((!rdm_packet_is_for_me) && (!rdm_packet_is_broadcast)) {
		// Ignore RDM packet
	} else if (command_class == E120_DISCOVERY_COMMAND) {

		if (param_id == E120_DISC_UNIQUE_BRANCH) {

			if (!rdm_muted) {

				if ((memcmp(rdm_cmd->param_data, uid_device, RDM_UID_SIZE) <= 0) && (memcmp(uid_device, rdm_cmd->param_data + 6, RDM_UID_SIZE) <= 0)) {
					monitor_line(MONITOR_LINE_STATUS, "E120_DISC_UNIQUE_BRANCH");

					struct _rdm_discovery_msg *p = (struct _rdm_discovery_msg *) (rdm_data);
					uint16_t rdm_checksum = 6 * 0xFF;

					uint8_t i = 0;
					for (i = 0; i < 7; i++) {
						p->header_FE[i] = 0xFE;
					}
					p->header_AA = 0xAA;

					for (i = 0; i < 6; i++) {
						p->masked_device_id[i + i] = uid_device[i] | 0xAA;
						p->masked_device_id[i + i + 1] = uid_device[i] | 0x55;
						rdm_checksum += uid_device[i];
					}

					p->checksum[0] = (rdm_checksum >> 8) | 0xAA;
					p->checksum[1] = (rdm_checksum >> 8) | 0x55;
					p->checksum[2] = (rdm_checksum & 0xFF) | 0xAA;
					p->checksum[3] = (rdm_checksum & 0xFF) | 0x55;

					rdm_send_discovery_respond_message(rdm_data, sizeof(struct _rdm_discovery_msg));
				}
			}
		} else if (param_id == E120_DISC_UN_MUTE) {
			monitor_line(MONITOR_LINE_STATUS, "E120_DISC_UN_MUTE");

			if (rdm_cmd->param_data_length != 0) {
				/* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
				 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
				 */
				//rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
				return;
			}
			rdm_muted = false;

			if (rdm_packet_is_for_me) {
				rdm_cmd->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
				rdm_cmd->param_data_length = 2;
				rdm_cmd->param_data[0] = 0x00;	// Control Field
				rdm_cmd->param_data[1] = 0x00;	// Control Field
				if (rdm_sub_devices_get()) {
					rdm_cmd->param_data[1] |= RDM_CONTROL_FIELD_SUB_DEVICE_FLAG;
				}
				rdm_send_respond_message_ack(rdm_data);
			}
		} else if (param_id == E120_DISC_MUTE) {
			monitor_line(MONITOR_LINE_STATUS, "E120_DISC_MUTE");

			if (rdm_cmd->param_data_length != 0) {
				/* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
				 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
				 */
				//rdm_send_respond_message_nack(E120_NR_FORMAT_ERROR);
				return;
			}

			rdm_muted = true;

			if (rdm_packet_is_for_me) {
				rdm_cmd->message_length = RDM_MESSAGE_MINIMUM_SIZE + 2;
				rdm_cmd->param_data_length = 2;
				rdm_cmd->param_data[0] = 0x00;	// Control Field
				rdm_cmd->param_data[1] = 0x00;	// Control Field
				if (rdm_sub_devices_get()) {
					rdm_cmd->param_data[1] |= RDM_CONTROL_FIELD_SUB_DEVICE_FLAG;
				}
				rdm_send_respond_message_ack(rdm_data);
			}
		}
	} else {
		uint16_t sub_device = (rdm_cmd->sub_device[0] << 8) + rdm_cmd->sub_device[1];
		rdm_handlers(rdm_data, rdm_packet_is_broadcast || rdm_packet_is_vendorcast, command_class, param_id, rdm_cmd->param_data_length, sub_device);
	}
}
