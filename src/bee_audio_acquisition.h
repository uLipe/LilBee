/*
 *  @file bee_audio_acquisition.h
 *  @brief module responsible to capture audio samples
 */

#ifndef __BEE_AUDIO_ACQUISITION_H
#define __BEE_AUDIO_ACQUISITION_H

/* define audio sample frequency Hz*/
#define AUDIO_SAMPLE_FREQ 	AUDIO_FREQUENCY_44K

/* define audio bit resolution */
#define AUDIO_BIT_RES		16

/* define the audio nbr of channels */
#define AUDIO_CHANNELS		1

/* define the audio window length s/ms  */
#define AUDIO_WINDOW_LEN	8

/** audio frame data structure */
typedef struct audio_frame {
	uint16_t *audio_buffer;
	uint32_t sample_rate;
	uint32_t size;
}audio_frame_t;


/**
 * 	@fn audio_acq_init()
 *  @brief inits the audio acquisition module (but does not start capture)
 *  @param
 *  @return
 */
void audio_acq_init(void);

/**
 * 	@fn audio_start_capture()
 *  @brief starts the audio capture
 *
 *  @param
 *  @return
 */
void audio_start_capture(void);

/**
 * 	@fn audio_stop_capture()
 *  @brief disable audio capture
 *
 *  @param
 *  @return
 */
void audio_stop_capture(void);

/**
 * 	@fn audio_get_current_frame()
 *  @brief gets the current and most recent available frame
 *
 *  @param
 *  @return
 */
audio_frame_t *audio_get_current_frame(void);

/**
 * 	@fn audio_handler()
 *  @brief audio appliation event handler
 *
 *  @param
 *  @return
 */
void audio_handler(system_event_t ev);


#endif
