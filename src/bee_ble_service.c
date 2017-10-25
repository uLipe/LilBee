/*
 *  @file bee_ble_service
 *  @brief little bee aggresivity level ble service
 */

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_def.h"
#include "event_queue.h"
#include "SensorTile_BlueNRG.h"

#include "hal_types.h"
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

#include "bee_ble_service.h"


static bee_service_status_t state = k_bee_disconnected;
static uint16_t bee_service_handle;
static uint16_t bee_char_aggro_handle;



/** internal functions */
static void ble_stack_init(void)
{
	const char BoardName[] = { "LilBee" };
	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	uint8_t hwVersion;
	uint16_t fwVersion;

	/* Initialize the BlueNRG SPI driver */
	BNRG_SPI_Init();

	/* Initialize the BlueNRG HCI */
	HCI_Init();

	/* Reset BlueNRG hardware */
	BlueNRG_RST();

	/* get the BlueNRG HW and FW versions */
	getBlueNRGVersion(&hwVersion, &fwVersion);


	/*
	 * Reset BlueNRG again otherwise it will fail.
	 */
	BlueNRG_RST();

	aci_gatt_init();

	if (hwVersion > 0x30) {
		aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07,
			&service_handle, &dev_name_char_handle,
			&appearance_char_handle);
	} else {
		aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle,
			&dev_name_char_handle, &appearance_char_handle);
	}


	aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
			sizeof(BoardName), (uint8_t *) BoardName);


	aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
	OOB_AUTH_DATA_ABSENT,
	NULL, 7, 16,
	USE_FIXED_PIN_FOR_PAIRING, 123456,
	BONDING);


	/* Set output power level */
	aci_hal_set_tx_power_level(1, 4);

	return;

}

static void ble_service_add(void)
{
	tBleStatus ret;

	uint8_t uuid[16];

	COPY_CONFIG_SERVICE_UUID(uuid);
	ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 1 + 3,
			&bee_service_handle);

	if (ret != BLE_STATUS_SUCCESS)
		goto fail;

	COPY_CONFIG_W2ST_CHAR_UUID(uuid);
	ret = aci_gatt_add_char(bee_service_handle, UUID_TYPE_128, uuid,
			4 /* Max Dimension */,
			CHAR_PROP_NOTIFY,
			ATTR_PERMISSION_NONE,
			GATT_DONT_NOTIFY_EVENTS, 16, 0,
			&bee_char_aggro_handle);

	if (ret != BLE_STATUS_SUCCESS) {
		goto fail;
	}

	return BLE_STATUS_SUCCESS;

fail:
	return BLE_STATUS_ERROR;
}

static void ble_start_advertisement(void)
{
	hci_le_set_scan_resp_data(0, NULL);

	char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'L','B'};
	aci_gap_set_discoverable(ADV_IND, 0, 0,
								PUBLIC_ADDR,
								NO_WHITE_LIST_USE,
								sizeof(local_name),
								local_name, 0, NULL, 0, 0);

}

static void ble_stop_advertisement(void)
{
	aci_gap_set_non_discoverable();
}

static void ble_bee_svc_char_update(void *val, uint8_t size)
{
	if(val && size) {
		aci_gatt_update_char_value(bee_service_handle, bee_char_aggro_handle,
				0,size,val);
	}
}

static void ble_update_aggro_level(void)
{
}

/** public functions */

void bee_ble_init(void)
{
	ble_stack_init();
	ble_service_add();
	ble_stop_advertisement();
}

bee_ble_retcode_t bee_ble_start_advertisement(void)
{
	bee_ble_retcode_t ret = k_bee_ok;

	if(k_bee_disconnected != state) {
		ret = k_bee_error;
		goto fail_cannot_advertise;
	}

	ble_start_advertise();

fail_cannot_advertise:
	return(ret);
}


bee_service_status_t bee_ble_get_state(void)
{
	return(state);
}

void bee_ble_handler(system_event_t ev)
{
	switch(ev) {
	case k_blehcievent:
		HCI_Process();
		break;

	case k_aggresivity_available:
		ble_update_aggro_level();
		break;

	case k_bleadvertising:
		state = k_bee_advertising;
		break;

	case k_bleconnected:
		state = k_bee_connected;
		break;

	case k_bledisconnected:
		state = k_bee_disconnected;
		break;
	}
}


/* external reserved functions */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch(GPIO_Pin){
    case BNRG_SPI_EXTI_PIN:
      HCI_Isr();
      event_queue_put(k_blehcievent);
    break;
  }
}


void HCI_Event_CB(void *pckt)
{
	hci_uart_pckt *hci_pckt = pckt;
	hci_event_pckt *event_pckt = (hci_event_pckt*) hci_pckt->data;
	evt_le_meta_event *evt = NULL;
	evt_le_connection_complete *cc = NULL;
	evt_blue_aci *blue_evt = NULL;

	if (hci_pckt->type != HCI_EVENT_PKT) {
		return;
	}

	switch (event_pckt->evt) {

	case EVT_DISCONN_COMPLETE:
		event_queue_put(k_bledisconnected);

		break;
	case EVT_LE_META_EVENT:
		evt = (void *) event_pckt->data;

		switch (evt->subevent) {
		case EVT_LE_CONN_COMPLETE:
			cc = (void *) evt->data;
			event_queue_put(k_bleconnected);
			break;
		}

		break;
	case EVT_VENDOR:
		blue_evt = (void*) event_pckt->data;

		switch (blue_evt->ecode) {
		case EVT_BLUE_GATT_READ_PERMIT_REQ:

			break;
		case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
			break;
		}

		break;
	}
}

