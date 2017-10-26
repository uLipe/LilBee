/*
 *  @file lilbee.h
 *  @brief master header file
 *
 */

#ifndef __LILBEE_H
#define __LILBEE_H


/** libc headers */
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


/** SoC headers */
#include "stm32l4xx.h"
#include "stm32l4xx_periph_conf.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_it.h"
#include "hal_types.h"


/** BLE headers */
#include "bluenrg_gatt_server.h"
#include "bluenrg_gap.h"
#include "string.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "hci_const.h"
#include "gp_timer.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_aci_const.h"
#include "hci.h"
#include "hci_le.h"
#include "hal.h"
#include "sm.h"
#include "bluenrg_utils.h"
#include "bluenrg_l2cap_aci.h"
#include "uuid_ble_service.h"


/** Audio headers */
#include "SensorTile_audio_in.h"

/** BSP headers */
#include "SensorTile_BlueNRG.h"


/** applications headers */
#include "event_queue.h"
#include "bee_dsp.h"
#include "bee_audio_acquisition.h"
#include "bee_ble_service.h"


/** reference doc */


/**
 * 	@fn func()
 *  @brief
 *
 *  @param
 *  @return
 */


#endif
