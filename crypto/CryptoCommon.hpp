/**
 * @file
 * @author bzy <bzy@mit.edu>
 *
 * @brief Common cryptographic definitions for the FogVault platform.
 *
 * @license Apache License 2.0 (see LICENSE)
 *
 */

#ifndef __FOGVAULT_CRYPTOCOMMON_HPP__
#define __FOGVAULT_CRYPTOCOMMON_HPP__

#include <sodium.h>

#include <exception>
#include <stdexcept>

//#include <QSharedPointer>

#include <cstdint>

void FVCryptoInit();

class FVExceptionBase : public std::runtime_error
{
public:
    FVExceptionBase(const std::string & extra, const std::string & desc = "unknown FogVault exception")
        : std::runtime_error(desc + ": " + extra)
    {

    }
};

class FVCryptoInitException : public FVExceptionBase
{
public:
    FVCryptoInitException(const std::string & extra, const std::string & desc = "FVCryptoInit exception")
        : FVExceptionBase(extra, desc)
    {

    }
};

class FVSecureObjectException : public FVExceptionBase
{
public:
    FVSecureObjectException(const std::string & extra, const std::string & desc = "FVSecureObject exception")
        : FVExceptionBase(extra, desc)
    {

    }
};

class FVSecureObjectInvalidLockIDException : public FVSecureObjectException
{
public:
    FVSecureObjectInvalidLockIDException(const std::string & extra = "invalid lock ID")
        : FVSecureObjectException(extra)
    {

    }
};

class FVCryptoGenericException : public FVExceptionBase
{
public:
    FVCryptoGenericException(const std::string & extra, const std::string & desc = "FogVault crypto exception")
        : FVExceptionBase(extra, desc)
    {

    }
};

class FVCryptoDecryptVerifyException : public FVCryptoGenericException
{
public:
    FVCryptoDecryptVerifyException(const std::string & extra = "bad decrypt")
        : FVCryptoGenericException(extra)
    {

    }
};

class FVCryptoSignatureVerifyException : public FVCryptoGenericException
{
public:
    FVCryptoSignatureVerifyException(const std::string & extra = "bad signature")
        : FVCryptoGenericException(extra)
    {

    }
};

/// Memory-block class for key material (includes unlocking and locking).
///
/// @warning    DO NOT USE to wrap any classes that perform dynamic memory allocation!
///             This class cannot protect them correctly.
template <typename T>
class FVSecureObject
{
private:
    size_t len;
    uint32_t lock_id;
    bool mode_writable;
    T * mem;

    FVSecureObject(const FVSecureObject & other); // don't allow ordinary copying
    FVSecureObject & operator= (const FVSecureObject & other); // don't allow value assign

public:
    /// Creates a new key string (all zeroes).
    /// This starts locked.
    FVSecureObject()
    {
        size_t real_len = sizeof(T);
        len = real_len;
        mem = (T*)sodium_malloc(real_len);
        new(mem) T;
        sodium_mprotect_noaccess(mem);

        lock_id = 0xffffffff;
        mode_writable = false;
    }

    /// Securely deallocates this key string.
    ~FVSecureObject()
    {
        sodium_mprotect_readwrite(mem);
        mem->~T();
        sodium_free(mem);
        len = 0;
        lock_id = 0;
    }

    /* Unlocks the keystring for writing.

    @note The identifier mechanism here is to enforce good programming practice
    and is not really intended to provide security against remote code exec.

    @return an identifier that must be passed to data().
    */
    uint32_t UnlockRW()
    {
        lock_id = randombytes_random();
        mode_writable = true;
        sodium_mprotect_readwrite(mem);
        return lock_id;
    }

    /* Unlocks the keystring for reading.

    @note The identifier mechanism here is to enforce good programming practice
    and is not really intended to provide security against remote code exec.

    @return an identifier that must be passed to data().
    */
    uint32_t UnlockRO()
    {
        lock_id = randombytes_random();
        mode_writable = false;
        sodium_mprotect_readonly(mem);
        return lock_id;
    }

    /* Locks the keystring, invalidating any outstanding identifiers.
    */
    void Lock(uint32_t id)
    {
        if (id != lock_id)
        {
            throw FVSecureObjectInvalidLockIDException();
        }
        mode_writable = false;
        sodium_mprotect_noaccess(mem);
    }

    /* Fetches the length of this key material. */
    size_t length()
    {
        return len;
    }

    /* Fetches a mutable pointer to the underlying data. */
    T * data(uint32_t id)
    {
        if ((id != lock_id) | (!mode_writable))
        {
            throw FVSecureObjectInvalidLockIDException();
        }
        return mem;
    }

    /* Fetches an immutable pointer to the underlying data. */
    const T * const_data(uint32_t id) const
    {
        if (id != lock_id)
        {
            throw FVSecureObjectInvalidLockIDException();
        }
        return (const T *) mem;
    }

    /* Fetches a reference to the underlying object. */
    T & ref(uint32_t id)
    {
        if ((id != lock_id) | (!mode_writable))
        {
            throw FVSecureObjectInvalidLockIDException();
        }
        return *mem;
    }

    /* Fetches a read-only reference to the underlying object. */
    const T & const_ref(uint32_t id) const
    {
        if (id != lock_id)
        {
            throw FVSecureObjectInvalidLockIDException();
        }
        return (const T *) mem;
    }
};

#endif
