/*
 *  @file bee_audio_acquisition.c
 *  @brief module responsible to capture audio samples
 */

#include "lilbee.h"




/** internal variables */
static uint16_t audio_ping_pong_buffer [(AUDIO_SAMPLE_FREQ * AUDIO_CHANNELS)/AUDIO_WINDOW_LEN] = {0};
static uint16_t audio_frame_ping_pong_buffer[AUDIO_FRAME_SIZE] = {0};
static bool audio_active = false;
static bool audio_started = false;
static audio_frame_t framebuffer;
static uint32_t frame_index = 0;

/** internal functions */

/**
 * 	@fn on_audio_block()
 *  @brief handles new audio incoming block
 *
 *  @param
 *  @return
 */
static void on_audio_block(void)
{
	/* fills the audio frame buffer */
	memcpy(&audio_frame_ping_pong_buffer[frame_index / (sizeof(uint16_t))],
			&audio_ping_pong_buffer, sizeof(audio_ping_pong_buffer));

	frame_index += sizeof(audio_ping_pong_buffer);
	if(frame_index >= AUDIO_FRAME_SIZE) {
		/* broadcast frame available event and reset environment */
		BSP_AUDIO_IN_Stop();
		audio_active = true;
		frame_index = 0;
		event_queue_put(k_dsp_incoming_audio_available);
	}
}


/**
 * 	@fn on_audio_start()
 *  @brief handles starting capture audio event
 *
 *  @param
 *  @return
 */
static void on_audio_start(void)
{
	audio_active = true;
}

/**
 * 	@fn on_audio_stop()
 *  @brief handles stopping event
 *
 *  @param
 *  @return
 */
static void on_audio_stop(void)
{
	audio_active = false;
}



/** public functions */

void audio_acq_init(void)
{
	BSP_AUDIO_IN_Init(AUDIO_SAMPLE_FREQ, AUDIO_BIT_RES, AUDIO_CHANNELS);
	audio_started = true;
}

void audio_start_capture(void)
{
	BSP_AUDIO_IN_Record(&audio_ping_pong_buffer[0], 0);

	/* broadcast the event */
	event_queue_put(k_audiostartedcapture);
}

void audio_stop_capture(void)
{
	BSP_AUDIO_IN_Stop();

	/* broadcast the stopping action */
	event_queue_put(k_audiostoppedcapture);
}

audio_frame_t *audio_get_current_frame(void)
{
	audio_frame_t *ret = NULL;

	if(!audio_active)
		goto exit_no_audio;

	/* prevent multiple access */
	audio_active = false;
	__disable_irq();

	/* take the always the buffer is not being useed to acquire audio */
	ret = &framebuffer;
	ret->sample_rate = AUDIO_SAMPLE_FREQ;
	ret->size = sizeof(audio_frame_ping_pong_buffer)/2;
	ret->audio_buffer=&audio_frame_ping_pong_buffer[0];

	__enable_irq();
	audio_active = true;

exit_no_audio:
	return(ret);
}

void audio_handler(system_event_t ev)
{
	if(!audio_started)
		return;

	switch(ev) {
	case k_audioblockevent:
		on_audio_block();
		break;

	case k_audiostartedcapture:
		on_audio_start();
		break;

	case k_audiostoppedcapture:
		on_audio_stop();
		break;
	}
}


/**
 * 	@fn BSP_AUDIO_IN_TransferComplete_CallBack()
 *  @brief IRQ called by audio internals, defers the audio block event
 *
 *  @param
 *  @return
 */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
	event_queue_put(k_audioblockevent);
}




