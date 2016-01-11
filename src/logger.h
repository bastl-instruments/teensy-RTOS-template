#ifndef _LOGGER_H_
#define _LOGGER_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace Log {
	typedef enum {
		LOG_FATAL,
		LOG_ERROR,
		LOG_WARNING,
		LOG_INFO,
		LOG_DEBUG
	} loglevel_t;

#define LOG_PRINT(level, format, ...) { Log::print(level, __FILE__, __LINE__, format, __VA_ARGS__); }

	void print(loglevel_t lvl, const char *file, int line, const char *fmt, ...);
}



#endif

#endif /* _LOGGER_H_ */

