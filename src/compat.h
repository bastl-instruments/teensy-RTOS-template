#ifndef _COMPAT_H_
#define _COMPAT_H_
/* system includes */
/* local includes */

// compatibility functions to supress some errors from libc

#ifdef  __cplusplus
extern "C" {
#endif

int _getpid();
int _kill(int pid, int sig);
int _write();


#ifdef __cplusplus
}
#endif

#endif /* _COMPAT_H_ */

