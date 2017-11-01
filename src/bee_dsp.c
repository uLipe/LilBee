/*
 *  @file bee_dsp.c
 *  @brief module responsible to perform some DSP on the acquired audio
 */

#include "lilbee.h"


/** internal variables */
static uint32_t sample = 0;
static bee_spectra_t spectra = {0};
static float aggro_level = 0;

static bool dsp_lock = false;
static float dsp_float_buffer[1056];
extern const arm_rfft_fast_instance_f32 arm_rfft_fast_sR_f32_len512;

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
		dsp_float_buffer[i] = (float)audio_block->audio_buffer[i]/32768.0f - 1.0f;
		dsp_float_buffer[i+1] = (float)audio_block->audio_buffer[i+1]/32768.0f - 1.0f;
		dsp_float_buffer[i+2] = (float)audio_block->audio_buffer[i+2]/32768.0f - 1.0f;
		dsp_float_buffer[i+3] = (float)audio_block->audio_buffer[i+3]/32768.0f - 1.0f;
		dsp_float_buffer[i+4] = (float)audio_block->audio_buffer[i+4]/32768.0f - 1.0f;
		dsp_float_buffer[i+5] = (float)audio_block->audio_buffer[i+5]/32768.0f - 1.0f;
		dsp_float_buffer[i+6] = (float)audio_block->audio_buffer[i+6]/32768.0f - 1.0f;
		dsp_float_buffer[i+7] = (float)audio_block->audio_buffer[i+7]/32768.0f- 1.0f;
		dsp_float_buffer[i+8] = (float)audio_block->audio_buffer[i+8]/32768.0f - 1.0f;
		dsp_float_buffer[i+9] = (float)audio_block->audio_buffer[i+9]/32768.0f - 1.0f;
		dsp_float_buffer[i+10] = (float)audio_block->audio_buffer[i+10]/32768.0f - 1.0f ;
		dsp_float_buffer[i+11] = (float)audio_block->audio_buffer[i+11]/32768.0f - 1.0f;
		dsp_float_buffer[i+12] = (float)audio_block->audio_buffer[i+12]/32768.0f - 1.0f;
		dsp_float_buffer[i+13] = (float)audio_block->audio_buffer[i+13]/32768.0f - 1.0f;
		dsp_float_buffer[i+14] = (float)audio_block->audio_buffer[i+14]/32768.0f - 1.0f;
		dsp_float_buffer[i+15] = (float)audio_block->audio_buffer[i+15]/32768.0f - 1.0f;
	}



	/* prepare to compute the FFT */
	arm_rfft_fast_f32(&arm_rfft_fast_sR_f32_len512, dsp_float_buffer, &spectra.raw[0], 0);
	arm_cmplx_mag_f32(&spectra.raw[0], &spectra.raw[0], DSP_FFT_POINTS);


	spectra.spectral_points = DSP_FFT_POINTS;
	spectra.spectral_sample_rate = AUDIO_SAMPLE_FREQ;
	/* estimente the aggro level searching the hissing frequency interval */
	aggro_level = spectra.raw[32];

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

float bee_dsp_get_aggro_level(void)
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


