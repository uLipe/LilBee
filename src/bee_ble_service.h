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


void bee_ble_init(void);
bee_ble_retcode_t bee_ble_start_advertisement(void);
bee_service_status_t bee_ble_get_state(void);
void bee_ble_handler(system_event_t ev);

#endif
