/**
 * @file
 * @author bzy <bzy@mit.edu>
 *
 * @brief Common cryptographic definitions for the FogVault platform.
 *
 * @license Apache License 2.0 (see LICENSE)
 *
 */

#include "CryptoCommon.hpp"
#include <sodium.h>

void FVCryptoInit()
{
    if (sodium_init() == -1)
    {
        throw FVCryptoInitException("could not initialize libsodium");
    };
}
