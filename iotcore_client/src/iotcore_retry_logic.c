/*
* Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.
* The ASF licenses this file to You under the Apache License, Version 2.0
* (the "License"); you may not use this file except in compliance with
* the License.  You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "iotcore_retry_logic.h"
#include "iotcore_retry.h"

#include <stdlib.h>
#include <time.h>
#include <azure_c_shared_utility/tlsio.h>
#include <azure_c_shared_utility/threadapi.h>
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"
#include <azure_c_shared_utility/strings_types.h>
#include <azure_c_shared_utility/tickcounter.h>
#include <azure_c_shared_utility/doublylinkedlist.h>
#include "azure_umqtt_c/mqtt_client.h"

#define EPOCH_TIME_T_VALUE          0
#define EPOCH_TIME_T_VALUE          0
#define ERROR_TIME_FOR_RETRY_SECS   5       // We won't retry more than once every 5 seconds

static int retry_policy_exponential_backoff_with_jitter(int *permit, size_t* delay, void* retry_context_callback)
{
    int _result;

    if (retry_context_callback != NULL && permit != NULL && delay != NULL)
    {
        RETRY_LOGIC *_retry_logic = (RETRY_LOGIC*)retry_context_callback;
        int _num_of_failures = (int) _retry_logic->retry_count;
        *permit = 1;

        /*Intentionally not evaluating fraction of a second as it woudn't work on all platforms*/

        /* Exponential backoff with jitter
            1. delay = (pow(2, attempt) - 1 ) / 2
            2. delay with jitter = delay + rand_between(0, delay/2)
        */

        size_t _half_delta = ((1 << (size_t)_num_of_failures) - 1) / 4;
        if (_half_delta > 0)
        {
            *delay = _half_delta + (rand() % (int)_half_delta);
        }
        else
        {
            *delay = _half_delta;
        }

        _result = IOTCORE_ERR_OK;
    }
    else
    {
        _result = IOTCORE_ERR_ERROR;
    }
    return _result;
}

RETRY_LOGIC* create_retry_logic(IOTCORE_RETRY_POLICY retry_policy, size_t retry_timeout_limit_in_sec)
{
    RETRY_LOGIC *_retry_logic;
    _retry_logic = (RETRY_LOGIC*)malloc(sizeof(RETRY_LOGIC));
    if (_retry_logic != NULL)
    {
        _retry_logic->retry_policy = retry_policy;
        _retry_logic->retry_timeout_limit_in_sec = retry_timeout_limit_in_sec;
        switch (_retry_logic->retry_policy)
        {
            case IOTCORE_RETRY_NONE:
                // LogError("Not implemented chosing default");
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
            case IOTCORE_RETRY_IMMEDIATE:
                // LogError("Not implemented chosing default");
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
            case IOTCORE_RETRY_INTERVAL:
                // LogError("Not implemented chosing default");
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
            case IOTCORE_RETRY_LINEAR_BACKOFF:
                // LogError("Not implemented chosing default");
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
            case IOTCORE_RETRY_EXPONENTIAL_BACKOFF:
                // LogError("Not implemented chosing default");
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
            case IOTCORE_RETRY_RANDOM:
                // LogError("Not implemented chosing default");
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
            case IOTCORE_RETRY_EXPONENTIAL_BACKOFF_WITH_JITTER:
            default:
                _retry_logic->fn_retry_policy = &retry_policy_exponential_backoff_with_jitter;
                break;
        }
        _retry_logic->start = EPOCH_TIME_T_VALUE;
        _retry_logic->delay_from_last_connect_to_retry = 0;
        _retry_logic->last_connect = EPOCH_TIME_T_VALUE;
        _retry_logic->retry_started = IOT_FALSE;
        _retry_logic->retry_expired = IOT_FALSE;
        _retry_logic->first_attempt = IOT_TRUE;
        _retry_logic->retry_count = 0;
    }
    else
    {
        LogError("Init retry logic failed");
    }
    return _retry_logic;
}

void destroy_retry_logic(RETRY_LOGIC *retry_logic)
{
    if (retry_logic != NULL)
    {
        free(retry_logic);
        retry_logic = NULL;
    }
}

// Called when first attempted to re-connect
static void start_retry_timer(RETRY_LOGIC *retry_logic)
{
    if (retry_logic != NULL)
    {
        if (retry_logic->retry_started == false)
        {
            retry_logic->start = get_time(NULL);
            retry_logic->delay_from_last_connect_to_retry = 0;
            retry_logic->retry_started = true;
        }

        retry_logic->first_attempt = false;
    }
    else
    {
        LogError("Retry Logic parameter. NULL.");
    }
}

// Called when connected
void stop_retry_timer(RETRY_LOGIC *retry_logic)
{
    if (retry_logic != NULL)
    {
        if (retry_logic->retry_started == true)
        {
            retry_logic->retry_started = false;
            retry_logic->delay_from_last_connect_to_retry = 0;
            retry_logic->last_connect = EPOCH_TIME_T_VALUE;
            retry_logic->retry_count = 0;
        }
        else
        {
            LogError("Start retry logic before stopping");
        }
    }
    else
    {
        LogError("Retry Logic parameter. NULL.");
    }
}

// Called for every do_work when connection is broken
iot_bool can_retry(RETRY_LOGIC *retry_logic)
{
    iot_bool _result;
    time_t _now = get_time(NULL);

    if (retry_logic == NULL)
    {
        LogError("Retry Logic is not created, retrying forever");
        _result = IOT_TRUE;
    }
    else if (_now < 0 || retry_logic->start < 0)
    {
        LogError("Time could not be retrieved, retrying forever");
        _result = IOT_TRUE;
    }
    else if (retry_logic->retry_expired)
    {
        // We've given up trying to retry.  Don't do anything.
        _result = IOT_FALSE;
    }
    else if (retry_logic->first_attempt)
    {
        // This is the first time ever running through this code.  We need to try connecting no matter what.
        start_retry_timer(retry_logic);
        retry_logic->last_connect = _now;
        retry_logic->retry_count++;
        _result = IOT_TRUE;
    }
    else
    {
        // Are we trying to retry?
        if (retry_logic->retry_started)
        {
            // How long since we last tried to connect?  Store this in difftime.
            double _diff_time = get_difftime(_now, retry_logic->last_connect);

            // Has it been less than 5 seconds since we tried last?  If so, we have to
            // be careful so we don't hit the server too quickly.
            if (_diff_time <= ERROR_TIME_FOR_RETRY_SECS)
            {
                // As do_work can be called within as little as 1 ms, wait to avoid throtling server
                _result = IOT_FALSE;
            }
            else if (_diff_time < retry_logic->delay_from_last_connect_to_retry)
            {
                // delayFromLastConnectionToRetry is either 0 (the first time around)
                // or it's the backoff delta from the last time through the loop.
                // If we're less than that, don't even bother trying.  It's
                // Too early to retry
                _result = IOT_FALSE;
            }
            else
            {
                // last retry time evaluated have crossed, determine when to try next
                // In other words, it migth be time to retry, so we should validate with the retry policy function.
                int _permit = 0;
                size_t _delay;

                if (retry_logic->fn_retry_policy != NULL && 
                    (retry_logic->fn_retry_policy(&_permit, &_delay, retry_logic) == 0))
                {
                    // Does the policy function want us to retry (_permit == true), or are we still allowed to retry?
                    // (in other words, are we within retry_timeout_limit_in_sec seconds since starting to retry?)
                    // If so, see if we _really_ want to retry.
                    if ((_permit == 1) && ((retry_logic->retry_timeout_limit_in_sec == 0) || 
                        retry_logic->retry_timeout_limit_in_sec >= (_delay + get_difftime(_now, retry_logic->start))))
                    {
                        retry_logic->delay_from_last_connect_to_retry = _delay;

                        LogInfo("Evaluated _delay time %d sec.  Retry attempt count %d\n", 
                            _delay, retry_logic->retry_count);

                        // If the retry policy is telling us to connect right away ( <= ERROR_TIME_FOR_RETRY_SECS),
                        // or if enough time has elapsed, then we retry.
                        if ((retry_logic->delay_from_last_connect_to_retry <= ERROR_TIME_FOR_RETRY_SECS) ||
                            (_diff_time >= retry_logic->delay_from_last_connect_to_retry))
                        {
                            retry_logic->last_connect = _now;
                            retry_logic->retry_count++;
                            _result = IOT_TRUE;
                        }
                        else
                        {
                            // To soon to retry according to policy.
                            _result = IOT_FALSE;
                        }
                    }
                    else
                    {
                        // Retry expired.  Stop trying.
                        LogError("Retry timeout expired after %d attempts", retry_logic->retry_count);
                        retry_logic->retry_expired = true;
                        stop_retry_timer(retry_logic);
                        _result = IOT_FALSE;
                    }
                }
                else
                {
                    // We don't have a retry policy.  Sorry, can't even guess.  Don't bother even trying to retry.
                    LogError("Cannot evaluate the next best time to retry");
                    _result = IOT_FALSE;
                }
            }
        }
        else
        {
            // Since this function is only called when the connection is
            // already broken, we can start doing the rety logic.  We'll do the
            // actual interval checking next time around this loop.
            start_retry_timer(retry_logic);
            //wait for next do work to evaluate next best attempt
            _result = IOT_FALSE;
        }
    }
    return _result;
}
