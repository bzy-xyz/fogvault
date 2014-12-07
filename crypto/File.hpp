/**
 * @file
 * @author bzy <bzy@mit.edu>
 *
 * @brief Files for the FogVault platform.
 *
 * @license Apache License 2.0 (see LICENSE)
 *
 */

#ifndef __FOGVAULT_FILE_HPP__
#define __FOGVAULT_FILE_HPP__

#include <QFile>
#include <QDir>
#include <QString>
#include <QSharedPointer>
#include <QVector>
#include <QByteArray>

#include <string>

#include "CryptoCommon.hpp"
#include "UserKey.hpp"

#define FOGVAULT_FILE_VERSION 1

#define FOGVAULT_FILE_CTX_EXTENSION ".fvc"
#define FOGVAULT_FILE_MD_EXTENSION ".fvm"
#define FOGVAULT_FILE_MD_EXTENSION_LENGTH 4
#define FOGVAULT_FILE_BUNDLE_EXTENSION ".fvb"

#define FOGVAULT_FILENAME_REVNUM_LENGTH crypto_aead_chacha20poly1305_NPUBBYTES
#define FOGVAULT_FILE_REVNUM_LENGTH crypto_aead_chacha20poly1305_NPUBBYTES
#define FOGVAULT_FILE_MDSIG_LENGTH crypto_sign_BYTES
#define FOGVAULT_FILE_KTSIG_LENGTH crypto_sign_BYTES

#define FOGVAULT_FNEK_LENGTH crypto_secretbox_KEYBYTES
#define FOGVAULT_FEK_LENGTH crypto_secretbox_KEYBYTES

#define FOGVAULT_FNEK_ENC_LENGTH (FOGVAULT_FNEK_LENGTH + crypto_secretbox_MACBYTES)
#define FOGVAULT_FEK_ENC_LENGTH (FOGVAULT_FEK_LENGTH + crypto_secretbox_MACBYTES)

#define FOGVAULT_MD_MAGIC_LENGTH 8
#define FOGVAULT_MD_MAGIC "FVM_DATA"

#define FOGVAULT_BLOCK_ENC_LENGTH 4194304
#define FOGVAULT_BLOCK_LENGTH (FOGVAULT_BLOCK_ENC_LENGTH - crypto_aead_chacha20poly1305_ABYTES)

#define FOGVAULT_B64_OPTIONS QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals

/// a FogVault file metadata struct (opaque)
struct fv_file_metadata_t;
struct fv_file_control_t;

class FVFileOperationException : public FVExceptionBase
{
public:
    FVFileOperationException(const std::string & extra, const std::string & desc = "FVFile operation exception")
        : FVExceptionBase(extra, desc)
    {

    }
};

class FVFileInvalidStateException : public FVFileOperationException
{
public:
    FVFileInvalidStateException(const std::string & extra = "invalid state for file operation")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileMDVersionException : public FVFileOperationException
{
public:
    FVFileMDVersionException(const std::string & extra = "unsupported FogVault metadata version")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileMDReadException : public FVFileOperationException
{
public:
    FVFileMDReadException(const std::string & extra = "invalid or corrupt metadata file")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileMDInvalidSignatureException : public FVFileOperationException
{
public:
    FVFileMDInvalidSignatureException(const std::string & extra = "invalid metadata signature")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileUnknownPublicKeyException : public FVFileOperationException
{
public:
    FVFileUnknownPublicKeyException(const std::string & extra = "user public key unknown or out of range")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileInvalidPublicKeyException : public FVFileOperationException
{

public:
    FVFileInvalidPublicKeyException(const std::string & extra = "invalid user key or corrupt key table")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileReadAccessException : public FVFileOperationException
{
public:
    FVFileReadAccessException(const std::string & extra = "could not open file for reading")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileWriteAccessException : public FVFileOperationException
{
public:
    FVFileWriteAccessException(const std::string & extra = "could not open file for writing")
        : FVFileOperationException(extra)
    {

    }
};

class FVFileMDSizeException : public FVFileOperationException
{
public:
    FVFileMDSizeException(const std::string & extra = "metadata-indicated file size mismatch")
        : FVFileOperationException(extra)
    {

    }
};

/**
 * @brief A FogVault file (FogVault representation).
 *
 */
class FVFile
{
private:
    QString filename_pt;
    QString filename_enc;
    QString path_local;
    QSharedPointer<fv_file_metadata_t> md;
    QSharedPointer<fv_file_control_t> ctl;

    FVUserKeyPair & key;

    QString __decrypt(QString path_to, QString path_from);
    QString __encrypt(QString path_to, QString path_from);
    QString __writemd(QString path_to, FVUserKeyPair & k);

    void __verify(const FVUserPublicKey * owner_key);


public:
    /* Initialisation and cleanup */
    /// Creates a new FogVault file from a plaintext file.
    ///
    /// Looks for a corresponding metadata file in the same directory;
    /// if this is found, verifies that the provided keypair has permissions,
    /// otherwise creates a new metadata file with the given key as KT entry 0.
    ///
    /// @param owner_key key to verify file key table against (default: KT entry 0);
    ///        useful only if a metadata file exists, ignored otherwise
    FVFile(QFile & pt_file, FVUserKeyPair & key, const FVUserPublicKey * owner_key = NULL);

    /// Creates a new FogVault file to represent a directory with a plaintext name.
    ///
    /// Looks for a corresponding metadata file.
    /// If this is found, verifies that the provided keypair has permissions,
    /// otherwise creates a new metadata file with the given entry as KT entry 0.
    ///
    /// @param owner_key key to verify file key table against (default: KT entry 0);
    ///        useful only if a metadata file exists, ignored otherwise
    FVFile(QDir & dir, FVUserKeyPair & key, const FVUserPublicKey * owner_key = NULL);

    /**
    * @brief Loads a FogVault file from split metadata and PT/CT data,
    * decrypting using the given ECDH key.
    *
    * By default dat_file is assumed to be ciphertext; setting 'encrypted' to
    * false signals that dat_file is plaintext instead.
    */
    FVFile(QFile & md_file, QFile & dat_file, FVUserKeyPair & key, const FVUserPublicKey * owner_key = NULL, bool encrypted = true);

    /**
     * @brief Loads a FogVault file from metadata (directory or deleted file),
     * decrypting using the given ECDH key.
     *
     * @param reserved not used, set to 0
     * @param encrypted whether the filename is encrypted
     **/
    FVFile(QFile & md_file, uint32_t reserved, FVUserKeyPair & key, const FVUserPublicKey * owner_key = NULL, bool encrypted = true);

    /// Cleans up a FogVault file in preparation for deallocation.
    ~FVFile();

    /* File names */

    /// Gets the plaintext file name (not full path)
    QString PTFileName() const;

    /// Gets the ciphertext file name (not full path)
    QString CTFileName() const;

    /* File properties */

    /// Is this file marked as deleted?
    bool IsDeleted() const;

    /// Is this 'file' actually a directory?
    bool IsDirectory() const;

    void SetDeleted();

    /* Writing */

    /// Writes plaintext to a new file at the given directory.
    /// Returns the output filename.
    QString WritePT(QDir & dir);

    /// Writes plaintext to a new file (given full path).
    /// Returns the output filename.
    QString WritePT(const QString & path);

    /// Writes plaintext to a new temporary file.
    /// Returns the output filename.
    QString WritePT();

    /// Writes ciphertext to a new temporary file.
    /// Returns the output filename.
    QString WriteCT();

    /// Writes metadata to a new temporary file.
    /// Returns the output filename.
    ///
    QString WriteMD(bool encrypt_filename = true);

    /// Writes metadata to a new file at the given directory.
    /// Returns the output filename.
    ///
    /// @param k the metadata signing key to use
    QString WriteMD(QDir & dir, bool encrypt_filename = true);


    /* Key management */

    /// Returns a list of public keys with access to this file.
    QVector<FVUserPublicKey> ListFileKeys();

    /// Adds a key to the file.
    /// This conducts a soft rekey.
    void AddFileKey(const FVUserPublicKey & key, FVUserKeyPair & owner_key);

    /// Removes a key from the file (by key).
    /// This conducts a hard rekey if a key was removed.
    void RemoveFileKey(const FVUserPublicKey & key, FVUserKeyPair & owner_key);

    /// Removes a key from the file (by index).
    /// This conducts a hard rekey if a key was removed.
    void RemoveFileKey(unsigned int index, FVUserKeyPair & owner_key);



};

#endif
