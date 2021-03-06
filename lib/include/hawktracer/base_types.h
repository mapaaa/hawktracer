#ifndef HAWKTRACER_BASE_TYPES_H
#define HAWKTRACER_BASE_TYPES_H

#include <hawktracer/macros.h>

#include <inttypes.h>

HT_DECLS_BEGIN

/** An unsigned integer used for event klass identifiers. */
typedef uint32_t HT_EventKlassId;
/** An unsigned integer used for representing timestamps in nanoseconds. */
typedef uint64_t HT_TimestampNs;
/** An unsigned integer used for event identifiers. */
typedef uint64_t HT_EventId;
/** An unsigned integer used for representing duration in nanoseconds. */
typedef uint64_t HT_DurationNs;
/** A standard boolean type, possible values: #HT_TRUE, #HT_FALSE */
typedef int HT_Boolean;
/** A pointer to a list of HT_Event (or derived) objects. */
typedef uint8_t* TEventPtr;
/** An unsigned integer used for thread identifiers. */
typedef uint32_t HT_ThreadId;
/** An unsigned integer guaranteed to be 8 bits on all platforms. */
typedef uint8_t HT_Byte;

/** Defines @b true value for the HT_Boolean type. */
#define HT_TRUE 1
/** Defines @b false value for the HT_Boolean type. */
#define HT_FALSE 0

/** Defines an invalid klass identifier. */
#define HT_INVALID_KLASS_ID ((HT_EventKlassId)-1)

/** Defines list of possible errors returned by library functions. */
typedef enum
{
    /** No error */
    HT_ERR_OK = 0,
    /** Memory allocation failed. This is very serious error, and
     * most likely your entire application is broken at this point of time. */
    HT_ERR_OUT_OF_MEMORY,
    /** Try to register a timeline feature using ID which is already taken
     * by some other feature. */
    HT_ERR_FEATURE_ALREADY_REGISTERED,
    /** Try to register container with name already used in registry.
     * This error is used internally, and is never returned by public
     * API. TODO: consider removing it from public enum. */
    HT_ERR_LISTENER_CONTAINER_ALREADY_REGISTERED,
    /** Cannot open file. */
    HT_ERR_CANT_OPEN_FILE,
    /** Cannot start tcp server. This error might have many root causes,
     * some of them are: socket can't be open, library is not able to bind to a port. */
    HT_ERR_CANT_START_TCP_SERVER,
    /** Unable to create listener container. This is most likely caused by
     * #HT_ERR_OUT_OF_MEMORY occured internally in the library. */
    HT_ERR_CANT_CREATE_LISTENER_CONTAINER
} HT_ErrorCode;

HT_DECLS_END

#endif /* HAWKTRACER_BASE_TYPES_H */
