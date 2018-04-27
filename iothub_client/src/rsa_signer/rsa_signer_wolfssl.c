
#include <azure_c_shared_utility/xlogging.h>
#include "rsa_signer.h"

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/signature.h>

#include <azure_c_shared_utility/base64.h>

enum wc_SignatureType sig_type = WC_SIGNATURE_TYPE_RSA_W_ENC;
enum wc_HashType hash_type = WC_HASH_TYPE_SHA256;

const char * rsa_sha256_base64_signature(unsigned char* data, const char* pemPrivateKey) {
    int ret;
    RsaKey rsaKey;
    RNG rng;
    byte *sigBuf = NULL;
    byte *derBuf = NULL;
    int sigLen;
    int idx = 0;
    int derLen;

    ret = wc_InitRng(&rng);
    if (ret != 0) {
        LogError("Init RNG failed! %d\n", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    ret = wc_InitRsaKey(&rsaKey, NULL);
    if (ret != 0) {
        LogError("Init RSA key failed! %d\n", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    derBuf = (byte*) malloc(strlen(pemPrivateKey));
    ret = wolfSSL_KeyPemToDer(pemPrivateKey, strlen(pemPrivateKey), derBuf, strlen(pemPrivateKey), NULL);
    if (ret < 0) {
        LogError("failed to convert pem private key to der %d", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    derLen = ret;

    ret = wc_RsaPrivateKeyDecode(derBuf, &idx, &rsaKey, derLen);
    if (ret != 0) {
        LogError("decode private key fail %d", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    sigLen = wc_SignatureGetSize(sig_type, &rsaKey, (word32)sizeof(rsaKey));
    if(sigLen <= 0) {
        LogError("RSA Signature size check fail! %d\n", sigLen);
        ret = EXIT_FAILURE;
        goto exit;
    }
    sigBuf = malloc(sigLen);
    if(!sigBuf) {
        printf("RSA Signature malloc failed!\n");
        ret = EXIT_FAILURE;
        goto exit;
    }

    ret = wc_SignatureGenerate(
            hash_type, sig_type,
            data, (word32)strlen(data),
            sigBuf, &sigLen,
            &rsaKey, sizeof(rsaKey),
            &rng);

    if(ret < 0) {
        ret = EXIT_FAILURE;
        goto exit;
    }

    STRING_HANDLE base64Handle = Base64_Encode_Bytes(sigBuf, sigLen);
    const char* signature = STRING_c_str(base64Handle);
    free(base64Handle);

    exit:

    if(sigBuf) {
        free(sigBuf);
    }
    if (derBuf) {
        free(derBuf);
    }
    wc_FreeRsaKey(&rsaKey);
    wc_FreeRng(&rng);
    if (ret == EXIT_FAILURE) {
        return NULL;
    }
    return signature;
}

int verify_rsa_sha256_signature(unsigned char* data, const char* pemCert, const char* base64Signature) {
    int ret;
    RsaKey pubKey;
    BUFFER_HANDLE bufferHandle = NULL;
    byte *derBuf = NULL;
    int derLen;
    int idx = 0;
    WOLFSSL_X509* cert = NULL;
    WOLFSSL_EVP_PKEY* pubKeyTmp = NULL;

    ret = wc_InitRsaKey(&pubKey, NULL);
    if (ret != 0) {
        LogError("Init RSA key failed! %d\n", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    derBuf = (byte*) malloc(strlen(pemCert));
    ret = wolfSSL_CertPemToDer(pemCert, strlen(pemCert), derBuf, strlen(pemCert), 0);
    if (ret < 0) {
        LogError("failed to convert pem private key to der %d", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    derLen = ret;

    cert = wolfSSL_X509_d2i(&cert, derBuf, derLen);
    if (cert == NULL) {
        LogError("wolfssl load cert fail");
        ret = EXIT_FAILURE;
        goto exit;
    }

    pubKeyTmp = wolfSSL_X509_get_pubkey(cert);
    if (pubKeyTmp == NULL) {
        LogError("wolfssl extract pubkey from cert fail");
        ret = EXIT_FAILURE;
        goto exit;
    }

    ret = wc_RsaPublicKeyDecode((byte*)pubKeyTmp->pkey.ptr, &idx, &pubKey, pubKeyTmp->pkey_sz);
    if (ret != 0) {
        LogError("RSA public key import failed! %d\n", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }

    bufferHandle = Base64_Decoder(base64Signature);
    byte* signature = BUFFER_u_char(bufferHandle);
    size_t signatureSize;
    ret = BUFFER_size(bufferHandle, &signatureSize);
    if (ret != 0) {
        LogError("base64 decode signature fail %d", ret);
        ret = EXIT_FAILURE;
        goto exit;
    }
    ret = wc_SignatureVerify(
            hash_type, sig_type,
            data, strlen(data),
            signature, signatureSize,
            &pubKey, sizeof(pubKey));
    if (ret != 0) {
        LogError("signature verify fail with %d", ret);
    }

    exit:

    wc_FreeRsaKey(&pubKey);
    BUFFER_delete(bufferHandle);
    if (pubKeyTmp) {
        wolfSSL_EVP_PKEY_free(pubKeyTmp);
    }
    if (cert) {
        wolfSSL_X509_free(cert);
    }

    return ret;
}