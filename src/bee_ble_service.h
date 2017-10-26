/*
 *  @file bee_ble_service
 *  @brief little bee aggresivity level ble service
 */

#ifndef __BEE_BLE_SERVICE
#define __BEE_BLE_SERVICE

typedef enum {
	k_bee_ok = 0,
	k_bee_invalid_param,
	k_bee_error,
}bee_ble_retcode_t;

typedef enum {
	k_bee_advertising,
	k_bee_disconnected,
	k_bee_connected,
}bee_service_status_t;


/**
 * 	@fn bee_ble_init()
 *  @brief inits the ble stack and bee service
 *
 *  @param
 *  @return
 */
void bee_ble_init(void);


/**
 * 	@fn bee_ble_start_advertisement()
 *  @brief starts service advertisement
 *
 *  @param
 *  @return
 */
bee_ble_retcode_t bee_ble_start_advertisement(void);

/**
 * 	@fn bee_ble_get_state()
 *  @brief gets the BLE stack status
 *
 *  @param
 *  @return
 */
bee_service_status_t bee_ble_get_state(void);

/**
 * 	@fn bee_ble_handler()
 *  @brief bee service application loop
 *
 *  @param
 *  @return
 */
void bee_ble_handler(system_event_t ev);

#endif
