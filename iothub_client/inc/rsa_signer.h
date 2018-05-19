//
// Created by Zhu,Zhu(ACU IOT) on 2018/3/27.
//

#ifndef RSA_SIGNER_H
#define RSA_SIGNER_H

#include <azure_c_shared_utility/gballoc.h>

MOCKABLE_FUNCTION(, const char*, rsa_sha256_base64_signature, unsigned char*, data, const char*, pemPrivateKey);
MOCKABLE_FUNCTION(, int, verify_rsa_sha256_signature, unsigned char*, data, const char*, pemCert, const char*, base64Signature);

#endif //RSA_SIGNER_H
