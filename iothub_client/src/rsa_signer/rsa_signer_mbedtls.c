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

#include "mbedtls/platform.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include "mbedtls/x509_crt.h"
#include <string.h>
#include <azure_c_shared_utility/xlogging.h>

#include "rsa_signer.h"

int Base64Encode(const unsigned char* buffer, size_t length, char** b64text) { //Encodes a binary safe base 64 string
    char* dest = malloc(sizeof(char) * 1000);
    size_t len;
    int ret = mbedtls_base64_encode(dest, 1000, &len,  buffer, length);
    if (ret != 0) {
        LogError( " failed\n  ! base64 encode returned -0x%04x\n", -ret);
        free(dest);
        return ret;
    }
    dest[len] = '\0';
    *b64text = dest;
    return 0;
}

int Base64Decode(char* b64message, unsigned char** buffer, size_t* length) {
    *buffer = malloc(sizeof(char) * 1000);
    unsigned char* tmp = *buffer;
    int ret = mbedtls_base64_decode(*buffer, 1000, length, b64message, strlen(b64message));
    if (ret != 0) {
        free(*buffer);
        return -1;
    }
    tmp[*length] = '\0';
    return 0;
}

const char * rsa_sha256_base64_signature(unsigned char* data, const char* pemPrivateKey) {
    int ret;
    unsigned char hash[32];
    unsigned char buf[MBEDTLS_MPI_MAX_SIZE];

    // pemPrivateKey must ends with a \n
    if (pemPrivateKey[strlen(pemPrivateKey) - 1] != '\n') {
        LogError("private key in pem for mbedtls must ends with a '\\n'");
        return NULL;
    }

    if( ( ret = mbedtls_md(
            mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ), data, strlen(data),
             hash ) ) != 0 )
    {
        LogError( " failed\n  ! Could not read raw data\n");
        return NULL;
    }

    mbedtls_pk_context pk;
    mbedtls_pk_init( &pk );

    if ((ret = mbedtls_pk_parse_key(&pk, pemPrivateKey, strlen(pemPrivateKey) + 1, NULL, 0)))
    {
        LogError( " failed\n  ! mbedtls_pk_parse_keyfile returned -0x%04x\n", -ret );
        mbedtls_pk_free(&pk);
        return NULL;
    }

    mbedtls_rsa_context* pem_rsa = mbedtls_pk_rsa(pk);

    if( ( ret = mbedtls_rsa_check_privkey( pem_rsa ) ) != 0 )
    {
        LogError( " failed\n  ! mbedtls_rsa_check_privkey failed with -0x%0x\n", -ret );
        mbedtls_pk_free(&pk);
        mbedtls_rsa_free( pem_rsa );
        return NULL;
    }

    if( ( ret = mbedtls_rsa_pkcs1_sign( pem_rsa, NULL, NULL, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,
                                        20, hash, buf ) ) != 0 )
    {
        LogError( " failed\n  ! mbedtls_rsa_pkcs1_sign returned -0x%0x\n\n", -ret );
        mbedtls_pk_free(&pk);
        mbedtls_rsa_free( pem_rsa );
        return NULL;
    }

    char* base64Signature;
    ret = Base64Encode(buf, pem_rsa->len, &base64Signature);

    if (ret != 0) {
        mbedtls_pk_free(&pk);
        mbedtls_rsa_free( pem_rsa );
        return NULL;
    }

    mbedtls_pk_free(&pk);
    mbedtls_rsa_free( pem_rsa );

    return base64Signature;
}

int verify_rsa_sha256_signature(unsigned char* data, const char* pemCert, const char* base64Signature) {
    int ret;
    unsigned char hash[32];

    if (pemCert[strlen(pemCert) - 1] != '\n') {
        LogError("cert in pem for mbedtls must ends with a '\\n'");
        return -1;
    }

    // Base64 decode
    unsigned char* sig_buf;
    size_t sig_len;
    ret = Base64Decode((char *)base64Signature, &sig_buf, &sig_len);

    if (ret != 0) {
        LogError("base 64 decode signature %s fail", base64Signature);
        free(sig_buf);
        return -1;
    }

    if( ( ret = mbedtls_md(
            mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ), data, strlen(data), hash ) ) != 0 )
    {
        LogError( " failed\n  ! Could not get SHA256 hash of %s", data );
        return -1;
    }

    mbedtls_pk_context pk;
    mbedtls_pk_init( &pk );
    mbedtls_x509_crt chain;
    mbedtls_x509_crt_init( &chain );

    ret = mbedtls_x509_crt_parse( &chain, pemCert, strlen(pemCert) + 1);
    if (ret != 0) {
        LogError("parse cert %s fail", pemCert);
        free(sig_buf);
        mbedtls_x509_crt_free(&chain);
        mbedtls_pk_free(&pk);
        return -1;
    }

    mbedtls_rsa_context* rsa = mbedtls_pk_rsa(chain.pk);

    ret = mbedtls_rsa_pkcs1_verify( rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,
                                          MBEDTLS_MD_SHA256, 20, hash, sig_buf );

    free(sig_buf);
    mbedtls_x509_crt_free(&chain);
    mbedtls_pk_free(&pk);
    mbedtls_rsa_free( rsa );

    return ret;
}
