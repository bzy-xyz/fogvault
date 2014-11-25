/**
 * @file
 * @author bzy <bzy@mit.edu>
 *
 * @brief User keys for the FogVault platform.
 *
 * @license Apache License 2.0 (see LICENSE)
 *
 */

#include "UserKey.hpp"

#include <sodium.h>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QSharedPointer>
#include <QDataStream>

struct fv_user_seckey_t
{
    unsigned char seckey_sign[FOGVAULT_USERKEY_SIGN_SEC_LENGTH];
    unsigned char seckey_ecdh[FOGVAULT_USERKEY_ECDH_SEC_LENGTH];
};

struct fv_user_pubkey_t
{
    unsigned char pubkey_sign[FOGVAULT_USERKEY_SIGN_PUB_LENGTH];
    unsigned char pubkey_ecdh[FOGVAULT_USERKEY_ECDH_PUB_LENGTH];
};

#define FV_USERSECKEY_CRYPTED_LEN FOGVAULT_USERKEY_SIGN_SEC_LENGTH+FOGVAULT_USERKEY_ECDH_SEC_LENGTH+crypto_secretbox_MACBYTES

struct fv_user_seckey_ondisk_t
{
    unsigned char salt[crypto_pwhash_scryptsalsa208sha256_SALTBYTES];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char crypted[FV_USERSECKEY_CRYPTED_LEN];
};

static unsigned char * alloc_key_from_pwd(const QString & pwd, const unsigned char * const salt)
{
    QByteArray real_pwd = pwd.toUtf8();
    // derive a key from the given password
    unsigned char * k = (unsigned char*)sodium_malloc(crypto_secretbox_KEYBYTES);
    sodium_mprotect_readwrite(k);
    if(crypto_pwhash_scryptsalsa208sha256(
                k, crypto_secretbox_KEYBYTES, (const char * const)real_pwd.data(), real_pwd.length(),
                salt, crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE
                ) != 0)
    {
        sodium_free(k);
        real_pwd.fill(0);
        throw FVCryptoGenericException("could not derive key");
    }
    sodium_mprotect_readonly(k);
    real_pwd.fill(0);
    return k;
}

//------- FVUserKeyPair --------//

// Generate a new user key pair
FVUserKeyPair::FVUserKeyPair()
{
    // initialize the seckey and pubkey respectively
    this->__key = QSharedPointer<FVSecureObject<fv_user_seckey_t>>
                                                                   (new FVSecureObject<fv_user_seckey_t>());
    this->__key_pub = QSharedPointer<fv_user_pubkey_t>
                      (new fv_user_pubkey_t);
    // fetch a handle to the underlying seckey data (so we can populate it)
    auto lock_id = this->__key->UnlockRW();
    fv_user_seckey_t * sk = (fv_user_seckey_t *)this->__key->data(lock_id);

    // generate a signing key
    crypto_sign_keypair(this->__key_pub->pubkey_sign, sk->seckey_sign);
    // generate a ECDH key
    crypto_box_keypair(this->__key_pub->pubkey_ecdh, sk->seckey_ecdh);

    // seal the seckey to safeguard it while it is not needed
    this->__key->Lock(lock_id);
}

// Load an encrypted key pair from file
FVUserKeyPair::FVUserKeyPair(const QString & filename, const QString & password)
{
    // initialize the seckey and pubkey respectively
    this->__key = QSharedPointer<FVSecureObject<fv_user_seckey_t>>
                                                                   (new FVSecureObject<fv_user_seckey_t>());
    this->__key_pub = QSharedPointer<fv_user_pubkey_t>
                      (new fv_user_pubkey_t);


    fv_user_seckey_ondisk_t from_file;
    QFile f(filename);
    if(!(f.open(QIODevice::ReadOnly)))
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("could not open user key file for reading");
    }
    if(f.read((char *)from_file.salt, sizeof(from_file.salt)) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble reading user key file");
    }
    if(f.read((char *)from_file.nonce, sizeof(from_file.nonce)) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble reading user key file");
    }
    if(f.read((char *)from_file.crypted, sizeof(from_file.crypted)) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble reading user key file");
    }
    f.close();

    // reconstruct an encryption key
    unsigned char * k = alloc_key_from_pwd(password, from_file.salt);

    // fetch a handle to our underlying seckey data (so we can populate it)
    auto lock_id = this->__key->UnlockRW();
    unsigned char * sk = (unsigned char *)this->__key->data(lock_id);

    // decrypt and verify the key
    if(crypto_secretbox_open_easy(
                sk, from_file.crypted, sizeof(from_file.crypted),
                from_file.nonce, k
                ) == -1)
    {
        throw FVCryptoDecryptVerifyException();
    }

    sodium_free(k);

    // seal the seckey to safeguard it while it is not needed
    this->__key->Lock(lock_id);
}

FVUserKeyPair::~FVUserKeyPair()
{
}

uint32_t FVUserKeyPair::SecKeyUnlock()
{
    return this->__key->UnlockRO();
}

void FVUserKeyPair::SecKeyLock(uint32_t id)
{
    this->__key->Lock(id);
}

const unsigned char * FVUserKeyPair::GetSignPubKey() const
{
    return this->__key_pub->pubkey_sign;
}

const unsigned char * FVUserKeyPair::GetECDHPubKey() const
{
    return this->__key_pub->pubkey_ecdh;
}

const unsigned char * FVUserKeyPair::GetSignSecKey(uint32_t id) const
{
    return this->__key->data(id)->seckey_sign;
}

const unsigned char * FVUserKeyPair::GetECDHSecKey(uint32_t id) const
{
    return this->__key->data(id)->seckey_ecdh;
}

const QSharedPointer<FVUserPublicKey> FVUserKeyPair::GetPubKey() const
{
    QSharedPointer<FVUserPublicKey> ret (new FVUserPublicKey(this->__key_pub));
    return ret;
}

int FVUserKeyPair::SaveToFile(const QString & filename, const QString & password) const
{
    // create the on-disk representation of our data
    fv_user_seckey_ondisk_t to_write;

    // generate a salt for key derivation
    randombytes_buf(to_write.salt, sizeof(to_write.salt));
    // derive a key from the given password
    unsigned char * k = alloc_key_from_pwd(password, to_write.salt);

    // generate a nonce for encryption
    randombytes_buf(to_write.nonce, sizeof(to_write.nonce));

    // encrypt the key pair
    auto id = this->__key->UnlockRO();
    crypto_secretbox_easy(
                to_write.crypted, (const unsigned char *)this->__key->data(id), sizeof(fv_user_seckey_t),
                to_write.nonce, k
                );
    this->__key->Lock(id);

    // done with the key material, now free
    sodium_free(k);

    // write the result to file
    QFile f(filename);
    if(!(f.open(QIODevice::WriteOnly)))
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("could not open user key file for writing");
    }
    if(f.write((const char *)to_write.salt, sizeof(to_write.salt)) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble writing user key file");
    }
    if(f.write((const char *)to_write.nonce, sizeof(to_write.nonce)) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble writing user key file");
    }
    if(f.write((const char *)to_write.crypted, sizeof(to_write.crypted)) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble writing user key file");
    }
    f.close();

    return 0;
}

QSharedPointer<FVSecureObject<fv_ecdh_secret_t> >
FVUserKeyPair::ComputeECDHSharedSecret(const FVUserPublicKey & other, uint32_t id) const
{
    QSharedPointer<FVSecureObject<fv_ecdh_secret_t> > ret (new FVSecureObject<fv_ecdh_secret_t>);
    auto ret_id = ret->UnlockRW();
    this->ComputeECDHSharedSecret(ret->data(ret_id)->data, other, id);
    ret->Lock(id);
    return ret;
}

int FVUserKeyPair::ComputeECDHSharedSecret(unsigned char * sec_out,
                                           const FVUserPublicKey & other, uint32_t id) const
{
    return crypto_scalarmult(sec_out, this->GetECDHSecKey(id), other.GetECDHPubKey());
}


QByteArray FVUserKeyPair::ComputeSignature(const QByteArray & data, uint32_t id) const
{
    return this->ComputeSignature((unsigned char*)data.data(), data.length(), id);
}

QByteArray FVUserKeyPair::ComputeSignature(const unsigned char * data, const size_t len, uint32_t id) const
{
    QByteArray ret(crypto_sign_BYTES, 0);
    this->ComputeSignature((unsigned char *)ret.data(), data, len, id);
    return ret;
}

int FVUserKeyPair::ComputeSignature(unsigned char * sig_out,
                                    const unsigned char * data, const size_t len, uint32_t id) const
{
    return crypto_sign_detached(sig_out, NULL, data, len, this->GetSignSecKey(id));
}

void FVUserKeyPair::VerifySignature(const QByteArray & data, const QByteArray & sig) const
{
    this->GetPubKey()->VerifySignature(data, sig);
}

void FVUserKeyPair::VerifySignature(const unsigned char * data, const size_t len, const unsigned char * sig) const
{
    this->GetPubKey()->VerifySignature(data, len, sig);
}

//------- FVUserPublicKey --------//

FVUserPublicKey::FVUserPublicKey(const QSharedPointer<fv_user_pubkey_t> data)
{
    this->__key_pub = data;
}

/// Copies a public key.
FVUserPublicKey::FVUserPublicKey(const FVUserPublicKey & other)
{
    this->__key_pub = other.__key_pub;
}

/// Loads a public key from a pubkey file.
FVUserPublicKey::FVUserPublicKey(const QString & filename)
{
    this->__key_pub = QSharedPointer<fv_user_pubkey_t>(new fv_user_pubkey_t);

    QFile f(filename);
    if(!(f.open(QIODevice::ReadOnly)))
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("could not open user key file for reading");
    }
    if(f.read((char*)this->__key_pub->pubkey_sign, FOGVAULT_USERKEY_SIGN_PUB_LENGTH) < FOGVAULT_USERKEY_SIGN_PUB_LENGTH)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble reading user key file");
    }
    if(f.read((char*)this->__key_pub->pubkey_ecdh, FOGVAULT_USERKEY_ECDH_PUB_LENGTH) < FOGVAULT_USERKEY_ECDH_PUB_LENGTH)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble reading user key file");
    }
    f.close();
}

/// Loads a public key from its binary serialised form.
FVUserPublicKey::FVUserPublicKey(const QByteArray & data)
{
    if(data.length() != FOGVAULT_USERKEY_SIGN_PUB_LENGTH + FOGVAULT_USERKEY_ECDH_PUB_LENGTH)
    {
        throw FVUserPublicKeyUnreadableException("Could not read public key (bad serialized length)");
    }

    this->__key_pub = QSharedPointer<fv_user_pubkey_t>(new fv_user_pubkey_t);

    QByteArray k_sign = data.left(FOGVAULT_USERKEY_SIGN_PUB_LENGTH);
    QByteArray k_ecdh = data.right(FOGVAULT_USERKEY_ECDH_PUB_LENGTH);

    memcpy(this->__key_pub->pubkey_sign, k_sign.data(), k_sign.length());
    memcpy(this->__key_pub->pubkey_ecdh, k_ecdh.data(), k_ecdh.length());
}

/// Deallocates the public key.
FVUserPublicKey::~FVUserPublicKey()
{

}

/// Retrieves the public signing key.
const unsigned char * FVUserPublicKey::GetSignPubKey() const
{
    return this->__key_pub->pubkey_sign;
}
/// Retrieves the public ECDH key.
const unsigned char * FVUserPublicKey::GetECDHPubKey() const
{
    return this->__key_pub->pubkey_ecdh;
}

/// Verifies a signature over some data.
void FVUserPublicKey::VerifySignature(const QByteArray & data, const QByteArray & sig) const
{
    this->VerifySignature((const unsigned char *)data.data(), data.length(), (const unsigned char *)sig.data());
}

/// Verifies a signature over some data.
void FVUserPublicKey::VerifySignature(const unsigned char * data, size_t len, const unsigned char * sig) const
{
    if(crypto_sign_verify_detached(sig, data, len, this->GetSignPubKey()) != 0)
    {
        throw FVCryptoSignatureVerifyException();
    }
}

/// Serialises this public key.
QByteArray FVUserPublicKey::Serialize() const
{
    QByteArray ret;
    ret.append((const char*)this->__key_pub->pubkey_sign, FOGVAULT_USERKEY_SIGN_PUB_LENGTH);
    ret.append((const char*)this->__key_pub->pubkey_ecdh, FOGVAULT_USERKEY_ECDH_PUB_LENGTH);
    return ret;
}

/// Serialises this public key to a QDataStream.
int FVUserPublicKey::WriteToStream(QDataStream & s) const
{
    QByteArray d = this->Serialize();
    s.writeRawData(d.data(), d.length());

    return d.length();
}

int FVUserPublicKey::SaveToFile(const QString & filename) const
{
    QFile f(filename);
    if(!(f.open(QIODevice::WriteOnly)))
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("could not open pubkey file for writing");
    }
    if(f.write(this->Serialize()) < 0)
    {
        // TODO modify this exception throw to include requested filename (useful for debug)
        throw FVCryptoGenericException("trouble writing pubkey file");
    }
    f.close();

    return 0;
}

bool FVUserPublicKey::operator==(const FVUserPublicKey & other) const
{
    return (sodium_memcmp(this->__key_pub->pubkey_sign, other.__key_pub->pubkey_sign, FOGVAULT_USERKEY_SIGN_PUB_LENGTH))
            & (sodium_memcmp(this->__key_pub->pubkey_ecdh, other.__key_pub->pubkey_ecdh, FOGVAULT_USERKEY_ECDH_PUB_LENGTH));
}
