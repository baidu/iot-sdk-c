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

/* This file contains cert trust chain needed to communicate with Baidu (IoT Hub) */

#include "certs.h"

const char certificates[] =
// add trust external ca root
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFdDCCBFygAwIBAgIQJ2buVutJ846r13Ci/ITeIjANBgkqhkiG9w0BAQwFADBv\r\n"
"MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\r\n"
"ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\r\n"
"eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\r\n"
"gYUxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\r\n"
"BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMSswKQYD\r\n"
"VQQDEyJDT01PRE8gUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIICIjANBgkq\r\n"
"hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAkehUktIKVrGsDSTdxc9EZ3SZKzejfSNw\r\n"
"AHG8U9/E+ioSj0t/EFa9n3Byt2F/yUsPF6c947AEYe7/EZfH9IY+Cvo+XPmT5jR6\r\n"
"2RRr55yzhaCCenavcZDX7P0N+pxs+t+wgvQUfvm+xKYvT3+Zf7X8Z0NyvQwA1onr\r\n"
"ayzT7Y+YHBSrfuXjbvzYqOSSJNpDa2K4Vf3qwbxstovzDo2a5JtsaZn4eEgwRdWt\r\n"
"4Q08RWD8MpZRJ7xnw8outmvqRsfHIKCxH2XeSAi6pE6p8oNGN4Tr6MyBSENnTnIq\r\n"
"m1y9TBsoilwie7SrmNnu4FGDwwlGTm0+mfqVF9p8M1dBPI1R7Qu2XK8sYxrfV8g/\r\n"
"vOldxJuvRZnio1oktLqpVj3Pb6r/SVi+8Kj/9Lit6Tf7urj0Czr56ENCHonYhMsT\r\n"
"8dm74YlguIwoVqwUHZwK53Hrzw7dPamWoUi9PPevtQ0iTMARgexWO/bTouJbt7IE\r\n"
"IlKVgJNp6I5MZfGRAy1wdALqi2cVKWlSArvX31BqVUa/oKMoYX9w0MOiqiwhqkfO\r\n"
"KJwGRXa/ghgntNWutMtQ5mv0TIZxMOmm3xaG4Nj/QN370EKIf6MzOi5cHkERgWPO\r\n"
"GHFrK+ymircxXDpqR+DDeVnWIBqv8mqYqnK8V0rSS527EPywTEHl7R09XiidnMy/\r\n"
"s1Hap0flhFMCAwEAAaOB9DCB8TAfBgNVHSMEGDAWgBStvZh6NLQm9/rEJlTvA73g\r\n"
"JMtUGjAdBgNVHQ4EFgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQD\r\n"
"AgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAGBgRVHSAAMEQGA1UdHwQ9\r\n"
"MDswOaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVy\r\n"
"bmFsQ0FSb290LmNybDA1BggrBgEFBQcBAQQpMCcwJQYIKwYBBQUHMAGGGWh0dHA6\r\n"
"Ly9vY3NwLnVzZXJ0cnVzdC5jb20wDQYJKoZIhvcNAQEMBQADggEBAGS/g/FfmoXQ\r\n"
"zbihKVcN6Fr30ek+8nYEbvFScLsePP9NDXRqzIGCJdPDoCpdTPW6i6FtxFQJdcfj\r\n"
"Jw5dhHk3QBN39bSsHNA7qxcS1u80GH4r6XnTq1dFDK8o+tDb5VCViLvfhVdpfZLY\r\n"
"Uspzgb8c8+a4bmYRBbMelC1/kZWSWfFMzqORcUx8Rww7Cxn2obFshj5cqsQugsv5\r\n"
"B5a6SE2Q8pTIqXOi6wZ7I53eovNNVZ96YUWYGGjHXkBrI/V5eu+MtWuLt29G9Hvx\r\n"
"PUsE2JOAWVrgQSQdso8VYFhH2+9uRv0V9dlfmrPb2LjkQLPNlzmuhbsdjrzch5vR\r\n"
"pu/xO28QOG8=\r\n"
"-----END CERTIFICATE-----\r\n"
;

// add your client cert of the principal
const char client_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDGjCCAgKgAwIBAgIIJkjAEV084T4wDQYJKoZIhvcNAQELBQAwMDETMBEGA1UE\r\n"
"AwwKc2FuYm94cm9vdDEMMAoGA1UECwwDQkNFMQswCQYDVQQGEwJDTjAeFw0xODAz\r\n"
"MDYwNjMxMTVaFw0xOTAyMDYxMDIyMzBaMG0xDjAMBgNVBAoMBUJhaWR1MQswCQYD\r\n"
"VQQGEwJDTjFAMD4GA1UEAww3Y2VydHRlc3QudGVzdHlpbmtlLmViN2NjZWYyLTZj\r\n"
"YzMtNDNhNS05NmU1LWMxZGM4MmFmNmVmODEMMAoGA1UECwwDQkNFMIGfMA0GCSqG\r\n"
"SIb3DQEBAQUAA4GNADCBiQKBgQCb3MNQcMKYd92Ox0HmEcql5A3GwMVHpN2abHlq\r\n"
"/gvapP9K9xjSIHOFlS+za2I+Rbppup0aktPb+pqM9biCGApwHtSOhXDPjah9UYc9\r\n"
"poBFe09smyKsn4TTlYz9TqFVl8jKBx8XOF8EQG05ozDwXwfzVB250jU8bd6s4U3x\r\n"
"gTimYQIDAQABo38wfTAdBgNVHQ4EFgQUQK2ksqimvJ16g/bN0EHODkGJb5cwDAYD\r\n"
"VR0TAQH/BAIwADAfBgNVHSMEGDAWgBTNiNUSaVyEC9Ey8izvyWrHldubPDAOBgNV\r\n"
"HQ8BAf8EBAMCBeAwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMEMA0GCSqG\r\n"
"SIb3DQEBCwUAA4IBAQAhBiLLgujx8XYGnvcpy7WoYFoFwyK6c+EWMURv1EShEBq5\r\n"
"e11YZ/aUwhMnBz3IgwT8u+F80sDg04cBIqvcYP8yrqrWItwPI5fg1+VErZZbkPlu\r\n"
"mcW8Y0tCBI7T5q0Gl3wdvMGh/4+1FLB11+n1VooTakDYxzPnQCLV9DeRhRzuRq7O\r\n"
"8B2E/j0ZBwKvmxRRLnq8ZwakQsoNzUn5VW2tFKcETZ/A3/oBbdtQ5AGReDPcAC6b\r\n"
"7FV/Q/75B4KcKH9U4+hMd1M3ARzPJX4OkhO2Hf5gidxLNs4a4Yp8R+df2whOz8ap\r\n"
"iQpKE+U/T/BwRsXzm1IKpLgkZd7DJvwXaIxc/GVM\r\n"
"-----END CERTIFICATE-----\r\n"
;

// add your client private key of the principal
const char client_key[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIICXAIBAAKBgQCb3MNQcMKYd92Ox0HmEcql5A3GwMVHpN2abHlq/gvapP9K9xjS\r\n"
"IHOFlS+za2I+Rbppup0aktPb+pqM9biCGApwHtSOhXDPjah9UYc9poBFe09smyKs\r\n"
"n4TTlYz9TqFVl8jKBx8XOF8EQG05ozDwXwfzVB250jU8bd6s4U3xgTimYQIDAQAB\r\n"
"AoGACxjKpx2AHU7bbWDuZiz6GpmECZSL9y/bvzTd6CoyOdzpeDLjh4Jb9zTJ8qJA\r\n"
"mmJohUKOEOhHQTA0dLjB7DE9/OUbrYirGya45b1TzV3AyY4K6ltI8yKWIM64EjgR\r\n"
"N2NNnIIPnWdBCh+nFVpTV4yNG7Gz4pNcdrrIsHCilRiegAECQQDRM+OopCwo/002\r\n"
"i5633VUvGtz15W6UlijGprOuhZlaUInlBEp/6YmhOTzp/SaINjUntRh3sJCXeA3a\r\n"
"6Z0pLvIBAkEAvrpPG/f90fMCxkvntSEqhMzYKn1NNfyi3HU22qAZe2mOm63B61ls\r\n"
"GXxakzKWmeIURFpZoa0MAJGti+CO+8b0YQJAW0drtuJjDkROuVT0HL9q8pGjBXtk\r\n"
"41odUofb8HMEdV6cvBtCMkuArKLfzCyTim00hi3DDj4w6JYOXYz+8MA2AQJBAKq8\r\n"
"xgzXl9TWomk5khdHtXRknC6NNQ1bN7/6/jwAjk84U31xsuMojejStZKH+uGOzW3T\r\n"
"I+Hjs5be0mkhgV5K4IECQGzR55W10zR/3rSwdAHGuO85nei2SGuBnHqKGbLvs6h/\r\n"
"fx5g5WWdYvuI6LoEwq2RXVNQRC+i9YtABk+b970G0XA=\r\n"
"-----END RSA PRIVATE KEY-----\r\n"
;

const char bos_root_ca[] = "-----BEGIN CERTIFICATE-----\n"
    "MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n"
    "yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n"
    "ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n"
    "U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n"
    "ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n"
    "aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n"
    "MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n"
    "ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n"
    "biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n"
    "U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n"
    "aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n"
    "nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n"
    "t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n"
    "SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n"
    "BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n"
    "rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n"
    "NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n"
    "BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n"
    "BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n"
    "aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n"
    "MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n"
    "p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n"
    "5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n"
    "WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n"
    "4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n"
    "hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n"
    "-----END CERTIFICATE-----";