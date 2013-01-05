/**
 * @file    logcfg.h
 * @brief   Log configuration header.
 * @details Log configuration file. You may also use this file in order to
 *          override the log driver default settings.
 *
 * @addtogroup LOG_CFG
 * @{
 */

#ifndef _LOGCFG_H_
#define _LOGCFG_H_

#define LOG_USE_WCHAR           1
#define LOG_USE_64BIT           1
#define LOG_USE_UINTS           1
#define LOG_USE_INTS            1
#define LOG_USE_OCTALS          1
#define LOG_USE_FLOATS          1
#define LOG_USE_STRICT_FLOATS   0
#define LOG_USE_PRINTF          1
#define LOG_USE_PRINTFC         1

#define LOG_DEFINE_WCHAR

#define LOGT_INTMAX             LOGT_INT32
#define LOGT_UINTMAX            LOGT_UINT32
#define LOGT_OCTMAX             LOGT_OCT32
#define LOGT_HEXMAX             LOGT_HEX32

#define LOGT_INTPTRDIFF         LOGT_INT32
#define LOGT_UINTPTRDIFF        LOGT_UINT32
#define LOGT_OCTPTRDIFF         LOGT_OCT32
#define LOGT_HEXPTRDIFF         LOGT_HEX32

#define LOGT_INTSIZE            LOGT_INT32
#define LOGT_UINTSIZE           LOGT_UINT32
#define LOGT_OCTSIZE            LOGT_OCT32
#define LOGT_HEXSIZE            LOGT_HEX32

#endif /* _LOGCFG_H_ */

/** @} */
