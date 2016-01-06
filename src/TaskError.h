#ifndef _TASKERROR_H_
#define _TASKERROR_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace Tasks {

namespace Error {
//
// system-wide error function
//
void error(uint32_t errno);
int create();

}
}
#endif

#endif /* _TASKERROR_H_ */

