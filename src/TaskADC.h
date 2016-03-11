#ifndef _TASKADC_H_
#define _TASKADC_H_
/* system includes */
/* local includes */
#include <stdint.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace Tasks {

namespace ADC {
//
// system-wide error function
//
int create();
void setChannels(const uint16_t chans[], size_t nchans);

}
}
#endif

#endif /* _TASKADC_H_ */

