/**
 * @file
 * @author bzy <bzy@mit.edu>
 *
 * @brief User keys for the FogVault platform.
 *
 * @license Apache License 2.0 (see LICENSE)
 *
 */

#ifndef __FOGVAULT_USERKEY_HPP__
#define __FOGVAULT_USERKEY_HPP__

#include <sodium.h>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QSharedPointer>

#include "CryptoCommon.hpp"

#define FOGVAULT_USERKEY_SIGN_PUB_LENGTH crypto_sign_PUBLICKEYBYTES
#define FOGVAULT_USERKEY_SIGN_SEC_LENGTH crypto_sign_SECRETKEYBYTES
#define FOGVAULT_USERKEY_ECDH_PUB_LENGTH crypto_box_PUBLICKEYBYTES
#define FOGVAULT_USERKEY_ECDH_SEC_LENGTH crypto_box_SECRETKEYBYTES

#define FOGVAULT_USERKEY_PUB_LENGTH FOGVAULT_USERKEY_SIGN_PUB_LENGTH + FOGVAULT_USERKEY_ECDH_PUB_LENGTH
#define FOGVAULT_USERKEY_SEC_LENGTH FOGVAULT_USERKEY_SIGN_SEC_LENGTH + FOGVAULT_USERKEY_ECDH_SEC_LENGTH



/// raw secret key type (opaque)
struct fv_user_seckey_t;
/// raw public key type (opaque)
struct fv_user_pubkey_t;

/// data type for ECDH shared secrets
#define FOGVAULT_ECDH_SECRET_LENGTH crypto_scalarmult_BYTES
struct fv_ecdh_secret_t
{
    unsigned char data[FOGVAULT_ECDH_SECRET_LENGTH];
};

typedef QSharedPointer<FVSecureObject<fv_ecdh_secret_t> > FVECDHSecretPtr;


class FVUserPublicKey;
class FVUserKeyPair;

class FVUserKeyException : public FVExceptionBase
{
public:
    FVUserKeyException(const std::string & extra, const std::string & desc = "FVUserKey exception")
        : FVExceptionBase(extra, desc)
    {

    }
};

class FVUserKeyUnreadableException : public FVUserKeyException
{
public:
    FVUserKeyUnreadableException(const std::string & extra = "Could not read user key")
        : FVUserKeyException(extra)
    {

    }
};

class FVUserKeyBadDecryptException : public FVUserKeyException
{
public:
    FVUserKeyBadDecryptException(const std::string & extra = "Could not decrypt user key")
        : FVUserKeyException(extra)
    {

    }
};

class FVUserPublicKeyUnreadableException : public FVUserKeyException
{
public:
    FVUserPublicKeyUnreadableException(const std::string & extra = "Could not read public key")
        : FVUserKeyException(extra)
    {

    }
};

/**
 * @brief A user public key.
 */
class FVUserPublicKey
{
private:
    QSharedPointer<fv_user_pubkey_t> __key_pub;
public:
    /// Generates a new empty public key.
    FVUserPublicKey() {

    }

    /// Generates a new public key from the given data.
    FVUserPublicKey(const QSharedPointer<fv_user_pubkey_t> data);

    /// Copies a public key.
    FVUserPublicKey(const FVUserPublicKey & other);

    /// Loads a public key from a pubkey file.
    FVUserPublicKey(const QString & filename);

    /// Loads a public key from its binary serialised form.
    FVUserPublicKey(const QByteArray & data);

    /// Deallocates the public key.
    ~FVUserPublicKey();

    /// Retrieves the public signing key.
    const unsigned char * GetSignPubKey() const;
    /// Retrieves the public ECDH key.
    const unsigned char * GetECDHPubKey() const;

    /// Verifies a signature over some data.
    void VerifySignature(const QByteArray & data, const QByteArray & sig) const;

    /// Verifies a signature over some data.
    void VerifySignature(const unsigned char * data, size_t len, const unsigned char * sig) const;

    /// Serialises this public key.
    QByteArray Serialize() const;

    /// Serialises this public key to a QDataStream.
    /// @return the number of characters written.
    int WriteToStream(QDataStream & s) const;

    /// Saves this pubkey to a file.
    int SaveToFile(const QString & filename) const;

    /// Tests whether two FVUserPublicKey objects represent the same key.
    bool operator==(const FVUserPublicKey & other) const;
};

/**
 * @brief A user key pair.
 */
class FVUserKeyPair
{
private:
    //uint64_t lock_id;
    QSharedPointer<FVSecureObject<fv_user_seckey_t>> __key;
    QSharedPointer<fv_user_pubkey_t> __key_pub;

    FVUserKeyPair & operator= (const FVUserKeyPair & other); // disallow assign
public:
    /** @brief Generates a new user key pair.
    *
    * @note The secret key data is locked upon creation and must be unlocked
    *       before it can be used for sec-key operation.
    */
    FVUserKeyPair();

    /** @brief Loads a key pair from a file, optionally protected by a password.
    *
    * @note The secret key data is locked upon creation and must be unlocked
    *       before it can be used for sec-key operation.
    */
    FVUserKeyPair(const QString & filename, const QString & password);

    /**
     * @brief Copies an existing FVUserKeyPair.
     * @param other the FVUserKeyPair to copy
     *
     * @warning Copying user private keys is hazardous and should only be done
     * when absolutely necessary.
     */
    FVUserKeyPair(FVUserKeyPair & other);

    /// Deallocates a key pair.
    ~FVUserKeyPair();

    /// Unlocks a secret key READ-ONLY for sensitive operations.
    /// @return a lock ID required by GetFooSecKey().
    uint32_t SecKeyUnlock();
    /// Relocks a secret key after sensitive operations.
    void SecKeyLock(uint32_t id);

    /// Retrieves the public signing key.
    const unsigned char * GetSignPubKey() const;
    /// Retrieves the public ECDH key.
    const unsigned char * GetECDHPubKey() const;

    /// Retrieves the secret signing key (requires unlock).
    ///
    /// @param id a lock ID issued by SecKeyUnlock.
    const unsigned char * GetSignSecKey(uint32_t id) const;
    /// Retrieves the secret ECDH key (requires unlock).
    ///
    /// @param id a lock ID issued by SecKeyUnlock.
    const unsigned char * GetECDHSecKey(uint32_t id) const;

    /// Retrieves the public key as a distinct object.
    const QSharedPointer<FVUserPublicKey> GetPubKey() const;

    /// Saves the keypair to a file (by filename).
    int SaveToFile(const QString & filename, const QString & password) const;

    /* Crypto functions */

    /// Computes a shared secret for an ECDH handshake, given another public key.
    ///
    /// @note Requires key unlock.
    QSharedPointer<FVSecureObject<fv_ecdh_secret_t> >
    ComputeECDHSharedSecret(const FVUserPublicKey & other, uint32_t id) const;

    /// Computes a shared secret for an ECDH handshake, given another public key.
    /// @warning `sec_out` IS RED DATA and MUST be protected correctly!
    ///
    /// @note Requires key unlock.
    ///
    /// @param sec_out a FOGVAULT_ECDH_SECRET_LENGTH sized output array
    int ComputeECDHSharedSecret(unsigned char * sec_out, const FVUserPublicKey & other, uint32_t id) const;

    /// Computes a signature over some data.
    ///
    /// @note Requires key unlock.
    QByteArray ComputeSignature(const QByteArray & data, uint32_t id) const;

    /// Computes a signature over some data.
    ///
    /// @note Requires key unlock.
    QByteArray ComputeSignature(const unsigned char * data, const size_t len, uint32_t id) const;

    /// Computes a signature over some data.
    ///
    /// @note Requires key unlock.
    ///
    /// @param sig_out a crypto_sign_BYTES sized output array for the signature
    int ComputeSignature(unsigned char * sig_out, const unsigned char * data, const size_t len, uint32_t id) const;

    /// Verifies a signature over some data.
    void VerifySignature(const QByteArray & data, const QByteArray & sig) const;

    /// Verifies a signature over some data.
    void VerifySignature(const unsigned char * data, const size_t len, const unsigned char * sig) const;

};

#endif
