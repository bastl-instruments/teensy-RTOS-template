#ifndef _UTILS_H_
#define _UTILS_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _sine_nfo {
	_sine_nfo() : phase_acc(0), phase_inc(0), magnitude (16384) {}
	uint32_t phase_acc;
	uint32_t phase_inc;
	int32_t magnitude;
} sine_nfo_t;
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */

