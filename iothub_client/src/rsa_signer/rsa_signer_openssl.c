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


#include <azure_c_shared_utility/xlogging.h>
#include <openssl/err.h>
#include "openssl/pem.h"
#include "rsa_signer.h"

int Base64Encode(const unsigned char* buffer, size_t length, char** b64text) { //Encodes a binary safe base 64 string
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);

    char *buff = (char *)malloc(bufferPtr->length);
    memcpy(buff, bufferPtr->data, bufferPtr->length);
    buff[bufferPtr->length] = '\0';

    BIO_free_all(bio);

    *b64text = buff;

    return (0); //success
}

size_t calcDecodeLength(const char* b64input) { //Calculates the length of a decoded string
    size_t len = strlen(b64input);
    size_t padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
        padding = 2;
    else if (b64input[len-1] == '=') //last char is =
        padding = 1;

    return (len*3)/4 - padding;
}

int Base64Decode(char* b64message, unsigned char** buffer, size_t* length) { //Decodes a base64 encoded string
    BIO *bio, *b64;

    int decodeLen = calcDecodeLength(b64message);
    *buffer = (unsigned char*)malloc(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    bio = BIO_new_mem_buf(b64message, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
    *length = BIO_read(bio, *buffer, strlen(b64message));
    if(*length != decodeLen) //length should equal decodeLen, else something went horribly wrong
    {
        return -1;
    }
    BIO_free_all(bio);

    return (0); //success
}

const char * rsa_sha256_base64_signature(unsigned char* data, const char* pemPrivateKey) {
    int err;
    unsigned int sig_len;
    unsigned char sig_buf [4096];
    EVP_MD_CTX     md_ctx;
    EVP_PKEY *      pkey;

    ERR_load_crypto_strings();

    BIO_METHOD* bio_method = BIO_s_mem();
    if (bio_method == NULL)
    {
        LogError("failure in BIO_s_mem");
        return NULL;
    }

    BIO* cert_memory_bio = BIO_new(bio_method);

    if (cert_memory_bio == NULL)
    {
        LogError("failure in BIO_new");
        return NULL;
    }

    int puts_result = BIO_puts(cert_memory_bio, pemPrivateKey);

    if (puts_result < 0)
    {
        LogError("failure in BIO_puts");
        return NULL;
    }

    if ((size_t) puts_result != strlen(pemPrivateKey)) {
        LogError("mismatching legths");
        return NULL;
    }

    pkey = PEM_read_bio_PrivateKey(cert_memory_bio, NULL, NULL, NULL);
    if (pkey == NULL) {
        ERR_print_errors_fp (stderr);
        exit (1);
    }

    EVP_SignInit   (&md_ctx, EVP_sha256());
    EVP_SignUpdate (&md_ctx, data, strlen((const char *)data));
    sig_len = sizeof(sig_buf);
    err = EVP_SignFinal (&md_ctx, sig_buf, &sig_len, pkey);

    EVP_PKEY_free (pkey);
    EVP_MD_CTX_cleanup(&md_ctx);
    BIO_free(cert_memory_bio);

    if (err != 1) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    char* base64Signature;
    int result = Base64Encode(sig_buf, sig_len, &base64Signature);

    if (result != 0) {
        return NULL;
    }

    return base64Signature;
}


int verify_rsa_sha256_signature(unsigned char* data, const char* pemCert, const char* base64Signature) {
    int err;
    X509 *	x509;
    EVP_PKEY *      pkey;
    EVP_MD_CTX md_ctx;

    BIO_METHOD* bio_method = BIO_s_mem();
    if (bio_method == NULL)
    {
        LogError("failure in BIO_s_mem");
        return -1;
    }

    BIO* cert_memory_bio = BIO_new(bio_method);

    if (cert_memory_bio == NULL)
    {
        LogError("failure in BIO_new");
        return -1;
    }

    int puts_result = BIO_puts(cert_memory_bio, pemCert);

    if (puts_result < 0)
    {
        LogError("failure in BIO_puts");
        return -1;
    }

    if ((size_t) puts_result != strlen(pemCert)) {
        LogError("mismatching legths");
        return -1;
    }

    x509 = PEM_read_bio_X509(cert_memory_bio, NULL, NULL, NULL);

    if (x509 == NULL) {
        ERR_print_errors_fp (stderr);
        exit (1);
    }

    /* Get public key - eay */
    pkey = X509_get_pubkey(x509);
    if (pkey == NULL) {
        ERR_print_errors_fp (stderr);
        exit (1);
    }

    /* Verify the signature */
    EVP_VerifyInit   (&md_ctx, EVP_sha256());
    EVP_VerifyUpdate (&md_ctx, data, strlen((char*)data));

    /* Base64 decode */
    unsigned char* sig_buf;
    size_t sig_len;
    int res = Base64Decode((char *)base64Signature, &sig_buf, &sig_len);

    if (res != 0) {
        EVP_PKEY_free (pkey);
        EVP_MD_CTX_cleanup(&md_ctx);
        BIO_free(cert_memory_bio);
        X509_free(x509);
        return -1;
    }

    err = EVP_VerifyFinal (&md_ctx, sig_buf, (unsigned int) sig_len, pkey);

    EVP_PKEY_free (pkey);
    EVP_MD_CTX_cleanup(&md_ctx);
    BIO_free(cert_memory_bio);
    X509_free(x509);

    if (err != 1) {
        ERR_print_errors_fp (stderr);
        return -1;
    }

    return 0;
}