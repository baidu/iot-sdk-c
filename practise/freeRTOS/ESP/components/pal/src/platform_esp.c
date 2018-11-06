// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "sntp.h"

#ifdef USE_OPENSSL
#include "tlsio_pal.h"
#elif defined (USE_MBED_TLS)
#include "tlsio_mbedtls.h"
#endif

#include "azure_c_shared_utility/xlogging.h"

static const char* const ntpServer = "pool.ntp.org";

/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_004: [ The platform_init shall initialize the tlsio adapter. ] */
/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_005: [ The platform_init shall initialize the sntp client. ] */
int platform_init(void)
{
	// SNTP_SetServerName logs any necessary errors
	int result;
    if (SNTP_SetServerName(ntpServer) != 0)
    {
        LogError("Failed SNTP_SetServerName");
        result = __FAILURE__;
    }
    else
    {
    }
	if (SNTP_Init() != 0)
	{
        LogError("Failed SNTP_Init");
        result = __FAILURE__;
	}
    else
    {
        result = 0;
    }
    return result;
}

/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_008: [ The platform_get_default_tlsio shall return a set of tlsio functions provided by the OpenSSL micro tlsio implementation. ] */
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
#ifdef USE_OPENSSL
    return tlsio_pal_get_interface_description();
#elif defined (USE_MBED_TLS)
    return tlsio_mbedtls_get_interface_description();
#else
    LogError("no tlsio interface!");
    return NULL;
#endif
}

STRING_HANDLE platform_get_platform_info(void)
{
#ifdef USE_OPENSSL
    return STRING_construct("(openssl_compact)");
#elif defined (USE_MBED_TLS)
    return STRING_construct("(mbedtls)");
#else
    return  NULL;
#endif
}

/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_006: [ The platform_deinit shall deinitialize the sntp client. ] */
/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_007: [ The platform_deinit shall deinitialize the tlsio adapter. ] */
void platform_deinit(void)
{
	SNTP_Deinit();

	// The tlsio adapter for this platform does not need (or support) deinitialization
}
