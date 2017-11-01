/*
 *  @file bee_ble_service
 *  @brief little bee aggresivity level ble service
 */
#include "lilbee.h"

static bee_service_status_t state = k_bee_disconnected;
static uint16_t bee_service_handle;
static uint16_t bee_char_aggro_handle;
static bee_spectra_t bee_spectra;

/** internal functions */



/**
 * 	@fn ble_stack_init()
 *  @brief ups the BLE stack and starts the ACI interface
 *
 *  @param
 *  @return
 */
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

	/* inits the gatt server */
	aci_gatt_init();

	/* init the gap layer */
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


	/* set the security manager defaults */
	aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
	OOB_AUTH_DATA_ABSENT,
	NULL, 7, 16,
	USE_FIXED_PIN_FOR_PAIRING, 123456,
	BONDING);


	/* Set output power level */
	aci_hal_set_tx_power_level(1, 4);

	return;

}

/**
 * 	@fn ble_service_add()
 *  @brief Adds to ble the bee custom service
 *
 *  @param
 *  @return
 */
static void ble_service_add(void)
{
	tBleStatus ret;

	uint8_t uuid[16];

	/* creates the service  and add it to database */
	COPY_CONFIG_SERVICE_UUID(uuid);
	ret = aci_gatt_add_serv(UUID_TYPE_128, uuid, PRIMARY_SERVICE, 1 + 3,
			&bee_service_handle);


	/* creates the characteristic and adds it to database*/
	COPY_CONFIG_W2ST_CHAR_UUID(uuid);
	ret = aci_gatt_add_char(bee_service_handle, UUID_TYPE_128, uuid,
			4 /* Max Dimension */,
			CHAR_PROP_NOTIFY,
			ATTR_PERMISSION_NONE,
			GATT_DONT_NOTIFY_EVENTS, 16, 0,
			&bee_char_aggro_handle);

	(void)ret;
}



/**
 * 	@fn ble_start_advertisement()
 *  @brief instructs via ACI BLE to start advertisement
 *
 *  @param
 *  @return
 */
static void ble_start_advertisement(void)
{
	hci_le_set_scan_resp_data(0, NULL);

	char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'L','B'};
	aci_gap_set_discoverable(ADV_IND, 0, 0,
								PUBLIC_ADDR,
								NO_WHITE_LIST_USE,
								sizeof(local_name),
								local_name, 0, NULL, 0, 0);


	/* broadcast advertisement event */
	event_queue_put(k_bleadvertising);
}



/**
 * 	@fn bee_char_update()
 *  @brief ACI level bee service characteristic update
 *
 *  @param
 *  @return
 */
static void bee_char_update(void *val, uint8_t size)
{
	/* value and size must be valid */
	if(val && size) {

		/* send the new value through the gatt layer */
		aci_gatt_update_char_value(bee_service_handle, bee_char_aggro_handle,
				0,size,val);
	}
}


/**
 * 	@fn bee_ble_on_hci()
 *  @brief hci event, application level handler
 *
 *  @param
 *  @return
 */
static void bee_ble_on_hci(void)
{
	HCI_Process();
}


/**
 * 	@fn bee_ble_on_aggro()
 *  @brief gets the new agressivennes value and reports via BLE
 *
 *  @param
 *  @return
 */
static void bee_ble_on_aggro(void)
{
	if(state == k_bee_connected) {
		float aggro_value = bee_dsp_get_aggro_level();
		bee_char_update((uint8_t *)&aggro_value, sizeof(aggro_value));
	}
}

/**
 * 	@fn bee_ble_on_disconnected()
 *  @brief application level disconected handler
 *
 *  @param
 *  @return
 */
static void bee_ble_on_disconnected(void)
{
   state = k_bee_disconnected;
   bee_ble_start_advertisement();
}

/**
 * 	@fn bee_ble_on_connected()
 *  @brief application level connected handler
 *
 *  @param
 *  @return
 */
static void bee_ble_on_connected(void)
{
	state = k_bee_connected;
}

/**
 * 	@fn bee_ble_on_advertising()
 *  @brief application level advertise handler
 *
 *  @param
 *  @return
 */
static void bee_ble_on_advertising(void)
{
	state = k_bee_advertising;
}


/** public functions */

void bee_ble_init(void)
{

	/*
	 *  First we start the ble stack it brings up
	 *  the HCI and GATT servers, which enables
	 *  the addiction of our custom Bee service
	 *  finally start the advertisement
	 *
	 */
	ble_stack_init();
	ble_service_add();
	ble_start_advertisement();
}

bee_ble_retcode_t bee_ble_start_advertisement(void)
{
	bee_ble_retcode_t ret = k_bee_ok;

	/* cannot start advertisement with BLE connected
	 * or already advertising
	 */
	if(k_bee_disconnected != state) {
		ret = k_bee_error;
		goto fail_cannot_advertise;
	}


	ble_start_advertisement();

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
		bee_ble_on_hci();
		break;

	case k_aggresivity_available:
		bee_ble_on_aggro();
		break;

	case k_bleadvertising:
		bee_ble_on_advertising();
		break;

	case k_bleconnected:
		bee_ble_on_connected();
		break;

	case k_bledisconnected:
		bee_ble_on_disconnected();
		break;
	}
}


/* external reserved functions */


/**
 * 	@fn HAL_GPIO_EXTI_Callback()
 *  @brief HCI triansport IRQ  handler
 *
 *  @param
 *  @return
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin) {
	case BNRG_SPI_EXTI_PIN:
		HCI_Isr();
		event_queue_put(k_blehcievent);
		break;
	}
}



/**
 * 	@fn HCI_Event_CB()
 *  @brief HCI event handler
 *
 *  @param
 *  @return
 */
void HCI_Event_CB(void *pckt)
{

	/* maps the packet to one of the
	 * specified HCI events
	 */
	hci_uart_pckt *hci_pckt = pckt;
	hci_event_pckt *event_pckt = (hci_event_pckt*) hci_pckt->data;
	evt_le_meta_event *evt = NULL;
	evt_le_connection_complete *cc = NULL;
	evt_blue_aci *blue_evt = NULL;

	(void)cc;
	(void)blue_evt;

	/* is not a HCI packet, ignore it */
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

