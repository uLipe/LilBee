/*
 *  @file bee_dsp.c
 *  @brief module responsible to perform some DSP on the acquired audio
 */

#include "lilbee.h"


/** internal variables */
static uint32_t sample = 0;
static bee_spectra_t spectra = {0};
static int32_t aggro_level = 0;

static bool dsp_lock = false;


/** internal functions */

/**
 * 	@fn on_dsp_audio()
 *  @brief
 *
 *  @param
 *  @return
 */
static void on_dsp_audio(void)
{
	dsp_lock = true;

	/* broadcast the dsp end of processing */
	event_queue_put(k_dsp_endprocess);
}


/**
 * 	@fn on_dsp_endproc()
 *  @brief
 *
 *  @param
 *  @return
 */
static void on_dsp_endproc(void)
{
	dsp_lock = false;

	/* broadcast a new processed aggro level */
	event_queue_put(k_aggresivity_available);
}



/** public functions */

void bee_dsp_init(uint32_t dsp_sample)
{
	sample = dsp_sample;
}


uint32_t bee_dsp_get_sample_rate(void)
{
	return(sample);
}

uint32_t bee_dsp_get_aggro_level(void)
{
	uint32_t ret = 0;

	if(!dsp_lock) {
		ret = aggro_level;
	}

	return(ret);

}

bee_retcode_t bee_dsp_get_spectra(bee_spectra_t *raw)
{
	bee_ble_retcode_t ret = k_bee_err;

	if(!dsp_lock && raw) {

		dsp_lock = true;
		__disable_irq();
		memcpy(raw, &spectra, sizeof(spectra));
		__enable_irq();
		dsp_lock = false;

		ret = k_bee_ret_ok;
	}

	return(ret);
}

void bee_dsp_handler(system_event_t ev)
{
	switch(ev) {
	case k_dsp_incoming_audio_available:
		on_dsp_audio();
		break;

	case k_dsp_endprocess:
		on_dsp_endproc();
		break;
	}
}


