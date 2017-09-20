// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#ifndef SIMPLE_PUB_SAMPLE_H
#define SIMPLE_PUB_SAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

    int simple_pub_sample_run(const char * endpoint, 
    const char * username, 
    const char * password,
    const char * topic,
    const char * clientid,
    char useSsl);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_PUB_SAMPLE_H
