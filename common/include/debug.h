#ifndef __HOCO_DEBUG_H__
#define __HOCO_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <esp8266.h>

#define HOCO_VERBOSITY_DEBUG	5
#define HOCO_VERBOSITY_INFO		4
#define HOCO_VERBOSITY_WARN		3
#define HOCO_VERBOSITY_ERROR	2
#define HOCO_VERBOSITY_FATAL	1
#define HOCO_VERBOSITY_OFF		0

#ifndef HOCO_VERBOSITY
#define HOCO_VERBOSITY			HOCO_VERBOSITY_INFO
#endif

#ifndef HOCO_VERBOSITY_MQTT
#define HOCO_VERBOSITY_MQTT		HOCO_VERBOSITY_WARN
#endif

#if HOCO_VERBOSITY_MQTT >= HOCO_VERBOSITY_OFF
#include <mqttc.h>
#endif

#if HOCO_VERBOSITY_MQTT >= HOCO_VERBOSITY_DEBUG
#define DEBUG(fmt, ...) mqtt_debug("D:"fmt, ##__VA_ARGS__)
#elif HOCO_VERBOSITY >= HOCO_VERBOSITY_DEBUG
#define DEBUG(fmt, ...) ets_printf("D:"fmt"\r\n", ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#if HOCO_VERBOSITY_MQTT >= HOCO_VERBOSITY_INFO
#define INFO(fmt, ...) mqtt_debug("I:"fmt, ##__VA_ARGS__)
#elif HOCO_VERBOSITY >= HOCO_VERBOSITY_INFO
#define INFO(fmt, ...) ets_printf("I:"fmt"\r\n", ##__VA_ARGS__)
#else
#define INFO(fmt, ...)
#endif

#if HOCO_VERBOSITY_MQTT >= HOCO_VERBOSITY_WARN
#define WARN(fmt, ...) mqtt_debug("W:"fmt, ##__VA_ARGS__)
#elif HOCO_VERBOSITY >= HOCO_VERBOSITY_WARN
#define WARN(fmt, ...) ets_printf("W:"fmt"\r\n", ##__VA_ARGS__)
#else
#define WARN(fmt, ...)
#endif

#if HOCO_VERBOSITY_MQTT >= HOCO_VERBOSITY_ERROR
#define ERROR(fmt, ...) mqtt_debug("E:"fmt, ##__VA_ARGS__)
#elif HOCO_VERBOSITY >= HOCO_VERBOSITY_ERROR
#define ERROR(fmt, ...) ets_printf("E:"fmt"\r\n", ##__VA_ARGS__)
#else
#define ERROR(fmt, ...)
#endif

#if HOCO_VERBOSITY_MQTT >= HOCO_VERBOSITY_FATAL
#define FATAL(fmt, ...) mqtt_debug("F:"fmt, ##__VA_ARGS__)
#elif HOCO_VERBOSITY >= HOCO_VERBOSITY_FATAL
#define FATAL(fmt, ...) ets_printf("F:"fmt"\r\n", ##__VA_ARGS__)
#else
#define FATAL(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif
