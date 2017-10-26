/*
 *  @file bee_audio_acquisition.c
 *  @brief module responsible to capture audio samples
 */

#include "lilbee.h"

/** internal variables */
static uint16_t audio_ping_pong_buffer[2][(AUDIO_SAMPLE_FREQ * AUDIO_CHANNELS)/AUDIO_WINDOW_LEN] = {0};
const static uint32_t audio_buffer_size = sizeof(audio_ping_pong_buffer)/2;
static uint8_t active_buffer = 0;
static bool audio_active = false;
static bool audio_started = false;

audio_frame_t framebuffers[2];


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

}



/** public functions */

void audio_acq_init(void)
{
	BSP_AUDIO_IN_Init(AUDIO_SAMPLE_FREQ, AUDIO_BIT_RES, AUDIO_CHANNELS);
	audio_started = true;
}

void audio_start_capture(void)
{
	active_buffer = 0;
	BSP_AUDIO_IN_Record(&audio_ping_pong_buffer[active_buffer][0], audio_buffer_size);

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
	uint8_t buffer = 0;

	if(!audio_active)
		goto exit_no_audio;

	/* prevent multiple access */
	audio_active = false;
	__disable_irq();

	/* take the always the buffer is not being useed to acquire audio */
	buffer = ((active_buffer ^ 0x01) & 0x01);
	ret = &framebuffers[buffer];
	ret->sample_rate = AUDIO_SAMPLE_FREQ;
	ret->size = audio_buffer_size;
	ret->audio_buffer=&audio_ping_pong_buffer[buffer][0];

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


