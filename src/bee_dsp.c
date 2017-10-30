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
static float dsp_float_buffer[1056];


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

	/* gets the audio frame we will about to process */
	audio_frame_t *audio_block = audio_get_current_frame();

	/* no audio available or corrupted */
	if(audio_block == NULL)
		goto on_dsp_audio_exit;


	/* convert audio samples to float value */
	for(uint32_t i = 0; i < audio_block->size; i+=16) {
		/* unroll loop for performance */
		dsp_float_buffer[i] = (float)audio_block->audio_buffer[i];
		dsp_float_buffer[i+1] = (float)audio_block->audio_buffer[i+1];
		dsp_float_buffer[i+2] = (float)audio_block->audio_buffer[i+2];
		dsp_float_buffer[i+3] = (float)audio_block->audio_buffer[i+3];
		dsp_float_buffer[i+4] = (float)audio_block->audio_buffer[i+4];
		dsp_float_buffer[i+5] = (float)audio_block->audio_buffer[i+5];
		dsp_float_buffer[i+6] = (float)audio_block->audio_buffer[i+6];
		dsp_float_buffer[i+7] = (float)audio_block->audio_buffer[i+7];
		dsp_float_buffer[i+8] = (float)audio_block->audio_buffer[i+8];
		dsp_float_buffer[i+9] = (float)audio_block->audio_buffer[i+9];
		dsp_float_buffer[i+10] = (float)audio_block->audio_buffer[i+10];
		dsp_float_buffer[i+11] = (float)audio_block->audio_buffer[i+11];
		dsp_float_buffer[i+12] = (float)audio_block->audio_buffer[i+12];
		dsp_float_buffer[i+13] = (float)audio_block->audio_buffer[i+13];
		dsp_float_buffer[i+14] = (float)audio_block->audio_buffer[i+14];
		dsp_float_buffer[i+15] = (float)audio_block->audio_buffer[i+15];
	}

	/* prepare to compute the FFT */
	arm_cfft_f32(&arm_cfft_sR_f32_len512, dsp_float_buffer, 0, 1);
	arm_cmplx_mag_f32(dsp_float_buffer, &spectra.raw[0], DSP_FFT_POINTS);

	/* the spectra is ready, now we need to find the bee hissing frequency */
	uint32_t hiss_frequency = 2800;
	float accum = 0.0f;
	uint32_t iter = 0;
	for(uint32_t i = hiss_frequency; i < 3200; i += 45) {
		/* perform a simple average calculation */
		accum += bee_dsp_get_frequency_val(&spectra, i);
		iter++;
	}

	spectra.spectral_points = DSP_FFT_POINTS;
	spectra.spectral_sample_rate = AUDIO_SAMPLE_FREQ;

	/* estimente the aggro level searching the hissing frequency interval */
	aggro_level = (uint32_t)(accum / iter);

on_dsp_audio_exit:
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
	audio_start_capture();
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

	return(aggro_level);
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


