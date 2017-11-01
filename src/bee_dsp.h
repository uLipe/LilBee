/*
 *  @file bee_dsp.h
 *  @brief module responsible to perform some DSP on the acquired audio
 */

#ifndef __BEE_DSP_H
#define __BEE_DSP_H

/* number of points computed by the FFT */
#define DSP_FFT_POINTS	512


/* Bee audio RAW spectra */
typedef struct bee_spectra{
	uint32_t spectral_sample_rate;
	uint32_t spectral_points;
	float raw[DSP_FFT_POINTS];
}bee_spectra_t;





/**
 * 	@fn bee_dsp_init()
 *  @brief inits the lilbee digital processing appl
 *
 *  @param
 *  @return
 */
void bee_dsp_init(uint32_t dsp_sample);

/**
 * 	@fn bee_dsp_get_sample_rate()
 *  @brief gets the current sample rate used on dsp system
 *
 *  @param
 *  @return
 */
uint32_t bee_dsp_get_sample_rate(void);

/**
 * 	@fn bee_dsp_get_aggro_level()
 *  @brief gets the current agressivenes estimation
 *
 *  @param
 *  @return
 */
float bee_dsp_get_aggro_level(void);


/**
 * 	@fn bee_dsp_get_spectra()
 *  @brief gets the raw frequency spectrum from mic
 *
 *  @param
 *  @return
 */
bee_retcode_t bee_dsp_get_spectra(bee_spectra_t *raw);

/**
 * 	@fn bee_dsp_handler()
 *  @brief dsp main application handler
 *
 *  @param
 *  @return
 */
void bee_dsp_handler(system_event_t ev);



#endif
