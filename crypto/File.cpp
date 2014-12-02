/**
 * @file
 * @author bzy <bzy@mit.edu>
 *
 * @brief Files for the FogVault platform.
 *
 * @license Apache License 2.0 (see LICENSE)
 *
 */

#include "File.hpp"
#include "UserKey.hpp"

#include <ctime>
#include <cstdint>
#include <QVector>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>


enum fv_file_state_t
{
    FV_FILE_STATE_UNKNOWN,
    FV_FILE_STATE_MDONLY,
    FV_FILE_STATE_PT,
    FV_FILE_STATE_CT,
};

struct fv_fnek_t
{
    unsigned char data[FOGVAULT_FNEK_LENGTH];
};

struct fv_fek_t
{
    unsigned char data[FOGVAULT_FEK_LENGTH];
};

struct fv_file_keytable_entry_t
{
    unsigned char pk[FOGVAULT_USERKEY_PUB_LENGTH];
    unsigned char fnek_nonce[crypto_secretbox_NONCEBYTES];
    unsigned char fnek_enc[FOGVAULT_FNEK_ENC_LENGTH];
    unsigned char fek_nonce[crypto_secretbox_NONCEBYTES];
    unsigned char fek_enc[FOGVAULT_FEK_ENC_LENGTH];

    QSharedPointer<FVSecureObject<fv_fnek_t> > get_fnek(unsigned char * fpk, FVUserKeyPair & k) const
    {
        QSharedPointer<FVSecureObject<fv_fnek_t> > ret (new FVSecureObject<fv_fnek_t>);

        // complete D-H exchange
        FVUserPublicKey file_pubkey(QByteArray((const char *)fpk, FOGVAULT_USERKEY_PUB_LENGTH));
        auto k_id = k.SecKeyUnlock();
        QSharedPointer<FVSecureObject<fv_ecdh_secret_t> > dhsec = k.ComputeECDHSharedSecret(file_pubkey, k_id);
        k.SecKeyLock(k_id);

        // decrypt the fnek
        auto fn_id = ret->UnlockRW();
        auto dhs_id = dhsec->UnlockRO();

        if(crypto_secretbox_open_easy(ret->data(fn_id)->data, fnek_enc, sizeof(fnek_enc),
                                   fnek_nonce, dhsec->data(dhs_id)->data)
                == -1)
        {
            throw FVFileInvalidPublicKeyException();
        }

        dhsec->Lock(dhs_id);
        ret->Lock(fn_id);

        return ret;
    }

    QSharedPointer<FVSecureObject<fv_fek_t> > get_fek(unsigned char * fpk, FVUserKeyPair & k) const
    {
        QSharedPointer<FVSecureObject<fv_fek_t> > ret (new FVSecureObject<fv_fek_t>);

        // complete D-H exchange
        FVUserPublicKey file_pubkey(QByteArray((const char *)fpk, FOGVAULT_USERKEY_PUB_LENGTH));
        auto k_id = k.SecKeyUnlock();
        QSharedPointer<FVSecureObject<fv_ecdh_secret_t> > dhsec = k.ComputeECDHSharedSecret(file_pubkey, k_id);
        k.SecKeyLock(k_id);

        // decrypt the fek
        auto fn_id = ret->UnlockRW();
        auto dhs_id = dhsec->UnlockRO();

        if(crypto_secretbox_open_easy(ret->data(fn_id)->data, fek_enc, sizeof(fek_enc),
                                   fek_nonce, dhsec->data(dhs_id)->data)
                == -1)
        {
            throw FVFileInvalidPublicKeyException();
        }

        dhsec->Lock(dhs_id);
        ret->Lock(fn_id);

        return ret;
    }

    void set_pubkey(const FVUserPublicKey & k)
    {
        QByteArray d = k.Serialize();
        memcpy(pk, d.data(), FOGVAULT_USERKEY_PUB_LENGTH);
    }

    void rekey(FVUserKeyPair & filekey, FVSecureObject<fv_fnek_t> & fnek, FVSecureObject<fv_fek_t> & fek)
    {
        // compute shared secret
        FVUserPublicKey user_pubkey(QByteArray((const char *)this->pk, FOGVAULT_USERKEY_PUB_LENGTH));
        auto k_id = filekey.SecKeyUnlock();
        QSharedPointer<FVSecureObject<fv_ecdh_secret_t> > dhsec = filekey.ComputeECDHSharedSecret(user_pubkey, k_id);
        filekey.SecKeyLock(k_id);

        // generate new nonces
        randombytes_buf(fnek_nonce, sizeof(fnek_nonce));
        randombytes_buf(fek_nonce, sizeof(fek_nonce));

        auto dh_id = dhsec->UnlockRO();
        auto fn_id = fnek.UnlockRO();
        auto f_id = fek.UnlockRO();

        // encrypt and store the fnek
        crypto_secretbox_easy(this->fnek_enc, fnek.data(fn_id)->data, FOGVAULT_FNEK_LENGTH, fnek_nonce, dhsec->data(dh_id)->data);

        // encrypt and store the fek
        crypto_secretbox_easy(this->fek_enc, fek.data(f_id)->data, FOGVAULT_FEK_LENGTH, fek_nonce, dhsec->data(dh_id)->data);

        fek.Lock(f_id);
        fnek.Lock(fn_id);
        dhsec->Lock(dh_id);
    }

    void read_from_stream(QDataStream & s)
    {
        s.readRawData((char*)pk, sizeof(pk));
        s.readRawData((char*)fnek_nonce, sizeof(fnek_nonce));
        s.readRawData((char*)fnek_enc, sizeof(fnek_enc));
        s.readRawData((char*)fek_nonce, sizeof(fek_nonce));
        s.readRawData((char*)fek_enc, sizeof(fek_enc));
    }

    void write_to_stream(QDataStream & s) const
    {
        s.writeRawData((char*)pk, sizeof(pk));
        s.writeRawData((char*)fnek_nonce, sizeof(fnek_nonce));
        s.writeRawData((char*)fnek_enc, sizeof(fnek_enc));
        s.writeRawData((char*)fek_nonce, sizeof(fek_nonce));
        s.writeRawData((char*)fek_enc, sizeof(fek_enc));
    }
};

QDataStream & operator<< (QDataStream & s, const fv_file_keytable_entry_t & t)
{
    t.write_to_stream(s);
    return s;
}

QDataStream & operator>> (QDataStream & s, fv_file_keytable_entry_t & t)
{
    t.read_from_stream(s);
    return s;
}

struct fv_file_keytable_t
{
    unsigned char file_pubkey[FOGVAULT_USERKEY_PUB_LENGTH];
    QDateTime last_rekey_time;
    QVector<fv_file_keytable_entry_t> user_keys;
    unsigned char ktsig[FOGVAULT_FILE_KTSIG_LENGTH];

    QSharedPointer<FVSecureObject<fv_fnek_t> > __fnek;
    QSharedPointer<FVSecureObject<fv_fek_t> > __fek;

    QVector<FVUserPublicKey> list_keys() const
    {
        QVector<FVUserPublicKey> ret;
        for(QVector<fv_file_keytable_entry_t>::const_iterator i = user_keys.constBegin(); i != user_keys.constEnd(); i++)
        {
            FVUserPublicKey k(QByteArray((const char *)i->pk, FOGVAULT_USERKEY_PUB_LENGTH));
            ret.push_back(k);
        }
        return ret;
    }

    int length() const
    {
        return user_keys.length();
    }

    FVUserPublicKey key_by_index(uint i) const
    {
        if(i >= user_keys.length())
        {
            throw FVFileUnknownPublicKeyException();
        }
        return FVUserPublicKey(QByteArray((const char *)user_keys[i].pk, FOGVAULT_USERKEY_PUB_LENGTH));
    }

    int indexOf(const FVUserPublicKey & k)
    {
        return this->list_keys().indexOf(k);
    }

    QSharedPointer<FVSecureObject<fv_fnek_t> > get_fnek(FVUserKeyPair & k)
    {
        int i = indexOf(*(k.GetPubKey()));
        if(i == -1)
        {
            throw FVFileUnknownPublicKeyException();
        }
        QSharedPointer<FVSecureObject<fv_fnek_t> > ret = user_keys[i].get_fnek(file_pubkey, k);
        __fnek = ret;
        return ret;
    }

    QSharedPointer<FVSecureObject<fv_fek_t> > get_fek(FVUserKeyPair & k)
    {
        int i = indexOf(*(k.GetPubKey()));
        if(i == -1)
        {
            throw FVFileUnknownPublicKeyException();
        }
        QSharedPointer<FVSecureObject<fv_fek_t> > ret = user_keys[i].get_fek(file_pubkey, k);
        __fek = ret;
        return ret;
    }

    void cache_secret_keys(FVUserKeyPair & k)
    {
        this->get_fek(k);
        this->get_fnek(k);
    }

    void sign(FVUserKeyPair & k)
    {
        QByteArray a;

        QDataStream st_w (&a, QIODevice::WriteOnly);
        st_w.writeRawData((const char*)file_pubkey, sizeof(file_pubkey));
        st_w << last_rekey_time;
        st_w << user_keys;

        auto id = k.SecKeyUnlock();
        QByteArray sig = k.ComputeSignature(a, id);

        QDataStream sig_r (&sig, QIODevice::ReadOnly);
        sig_r.readRawData((char*)ktsig, sizeof(ktsig));
    }

    void verify(const FVUserPublicKey & k) const
    {
        QByteArray a;

        QDataStream st_w (&a, QIODevice::WriteOnly);
        st_w.writeRawData((const char *)file_pubkey, sizeof(file_pubkey));
        st_w << last_rekey_time;
        st_w << user_keys;

        QByteArray sig = QByteArray((const char *)ktsig, sizeof(ktsig));

        k.VerifySignature(a, sig);
    }

    void verify() const
    {
        auto k = key_by_index(0);
        verify(k);
    }

    /// Rekeys and signs the key table (retain fek/fnek).
    void rekey_soft(FVUserKeyPair & signing_key)
    {
        // just generate a new file key and rekey everything;
        // keeping the existing FEK and FNEK intact
        if(__fnek.isNull() | __fek.isNull())
        {
            __fnek = this->get_fnek(signing_key);
            __fek = this->get_fek(signing_key);
        }

        FVUserKeyPair fkp;

        for(QVector<fv_file_keytable_entry_t>::iterator i = user_keys.begin(); i != user_keys.end(); i++)
        {
            i->rekey(fkp, *__fnek, *__fek);
        }

        this->last_rekey_time = QDateTime::currentDateTimeUtc();

        this->sign(signing_key);
    }

    /// Rekeys and signs the key table (new fek/fnek).
    void rekey_hard(FVUserKeyPair & signing_key)
    {
        // generate new FEK and FNEK, and redo the whole thing
        FVUserKeyPair fkp;

        QSharedPointer<FVSecureObject<fv_fnek_t> > fnek;
        QSharedPointer<FVSecureObject<fv_fek_t> > fek;

        auto fn_d = fnek->UnlockRW();
        auto f_d = fnek->UnlockRW();

        randombytes_buf(fnek->data(fn_d)->data, FOGVAULT_FNEK_LENGTH);
        randombytes_buf(fek->data(f_d)->data, FOGVAULT_FEK_LENGTH);

        fnek->Lock(fn_d);
        fnek->Lock(f_d);

        for(QVector<fv_file_keytable_entry_t>::iterator i = user_keys.begin(); i != user_keys.end(); i++)
        {
            i->rekey(fkp, *fnek, *fek);
        }

        this->__fnek = fnek;
        this->__fek = fek;

        this->last_rekey_time = QDateTime::currentDateTimeUtc();

        this->sign(signing_key);
    }

    void add_key(const FVUserPublicKey & k, FVUserKeyPair & owner, bool rekey_hard = false)
    {
        if(this->indexOf(k) != -1)
        {
            return;
        }

        fv_file_keytable_entry_t to_add;

        to_add.set_pubkey(k);

        this->user_keys.append(to_add);

        if(rekey_hard)
        {
            this->rekey_hard(owner);
        }
        else
        {
            this->rekey_soft(owner);
        }
    }

    void rm_key(const FVUserPublicKey & k, FVUserKeyPair & owner)
    {
        if(this->indexOf(k) == -1)
        {
            throw FVFileUnknownPublicKeyException();
        }

        auto idx = this->indexOf(k);
        this->user_keys.remove(idx);

        this->rekey_hard(owner);
    }

    void read_from_stream(QDataStream & s)
    {
        s.readRawData((char*)file_pubkey, sizeof(file_pubkey));
        s >> last_rekey_time;
        s >> user_keys;
        s.readRawData((char*)ktsig, sizeof(ktsig));
    }

    void write_to_stream(QDataStream & s) const
    {
        s.writeRawData((char*)file_pubkey, sizeof(file_pubkey));
        s << last_rekey_time;
        s << user_keys;
        s.writeRawData((char*)ktsig, sizeof(ktsig));
    }
};

struct fv_file_metadata_t
{
    unsigned char magic[FOGVAULT_MD_MAGIC_LENGTH];
    quint32 version;
    unsigned char mdsig[FOGVAULT_FILE_MDSIG_LENGTH];
    qint64 pt_size;
    quint32 is_deleted;
    quint32 is_directory;
    unsigned char fn_revnum[FOGVAULT_FILENAME_REVNUM_LENGTH];
    unsigned char revnum[FOGVAULT_FILE_REVNUM_LENGTH];
    fv_file_keytable_t kt;

    fv_file_metadata_t()
        : version(FOGVAULT_FILE_VERSION)
    {
        memcpy(magic, FOGVAULT_MD_MAGIC, FOGVAULT_MD_MAGIC_LENGTH);
        sodium_memzero(mdsig, sizeof(mdsig));
        pt_size = 0;
        is_deleted = randombytes_random() & 0xfffffffe;
        is_directory = randombytes_random() & 0xfffffffe;
        randombytes_buf(fn_revnum, FOGVAULT_FILENAME_REVNUM_LENGTH);
        randombytes_buf(revnum, FOGVAULT_FILE_REVNUM_LENGTH);
    }

    quint32 num_blocks()
    {
        return (pt_size == 0 ? 0 : 1 + ((pt_size - 1) / FOGVAULT_BLOCK_LENGTH));
    }

    void incr_fn_revnum()
    {
        // HACK fn_revnum known to be exactly 8 bytes
        quint64 * n = (quint64 *)fn_revnum;
        (*n)++;
    }

    void incr_revnum(quint64 z = 1)
    {
        // HACK revnum known to be exactly 8 bytes
        quint64 * n = (quint64 *)revnum;
        (*n) += z;
    }

    void read_from_stream(QDataStream & s)
    {
        s.readRawData((char*)magic, FOGVAULT_MD_MAGIC_LENGTH);
        if(sodium_memcmp(magic, FOGVAULT_MD_MAGIC, FOGVAULT_MD_MAGIC_LENGTH) == -1)
        {
            throw FVFileMDReadException();
        }

        s >> version;
        if(version != FOGVAULT_FILE_VERSION)
        {
            throw FVFileMDVersionException();
        }

        s >> pt_size;
        s >> is_deleted;
        s >> is_directory;
        s.readRawData((char*)fn_revnum, sizeof(fn_revnum));
        s.readRawData((char*)revnum, sizeof(revnum));
        kt.read_from_stream(s);
        s.readRawData((char*)mdsig, sizeof(mdsig));
    }

    void write_to_stream(QDataStream & s, bool with_sig = true) const
    {
        s.writeRawData((const char*)magic, FOGVAULT_MD_MAGIC_LENGTH);
        s << version;
        s << pt_size;
        s << is_deleted;
        s << is_directory;
        s.writeRawData((const char*)fn_revnum, sizeof(fn_revnum));
        s.writeRawData((const char*)revnum, sizeof(revnum));
        kt.write_to_stream(s);
        if(with_sig)
        {
            s.writeRawData((const char*)mdsig, sizeof(mdsig));
        }
    }

    QByteArray adata(quint64 offset)
    {
        QByteArray ret;

        QDataStream st(&ret, QIODevice::WriteOnly);
        write_to_stream(st);
        st << offset;

        return ret;
    }

    void sign(FVUserKeyPair & k)
    {
        QByteArray a;

        QDataStream st_w (&a, QIODevice::WriteOnly);
        this->write_to_stream(st_w, false);

        auto id = k.SecKeyUnlock();
        QByteArray sig = k.ComputeSignature(a, id);

        QDataStream sig_r (&sig, QIODevice::ReadOnly);
        sig_r.readRawData((char*)mdsig, sizeof(mdsig));
    }

    bool verify(FVUserPublicKey & k) const
    {
        QByteArray a;

        QDataStream st_w (&a, QIODevice::WriteOnly);
        this->write_to_stream(st_w, false);

        QByteArray sig = QByteArray((const char*)mdsig, sizeof(mdsig));

        try
        {
            k.VerifySignature(a, sig);
        }
        catch (FVCryptoSignatureVerifyException & e)
        {
            return false;
        }

        return true;
    }

    bool verify_from_kt(int idx) const
    {
        auto k = this->kt.key_by_index(idx);
        return verify(k);
    }

    void verify() const
    {
        auto keys = this->kt.list_keys();
        for(auto k_i = keys.begin(); k_i != keys.end(); k_i++)
        {
            if(verify(*k_i))
            {
                return;
            }
        }
        throw FVFileMDInvalidSignatureException();
    }

    QString enc_fn(QString pt_fn)
    {
        if(this->kt.__fnek.isNull())
        {
            throw FVFileOperationException("API misuse: should cache FNEK before computing encrypted FN");
        }

        QByteArray pt_utf8 = pt_fn.toUtf8();
        QByteArray ct_utf8(pt_utf8.length() + crypto_secretbox_MACBYTES, 0);

        auto fnek_id = this->kt.__fnek->UnlockRO();

        crypto_secretbox_easy((unsigned char*)ct_utf8.data(), (unsigned char*)pt_utf8.data(), pt_utf8.length(), this->fn_revnum, this->kt.__fnek->data(fnek_id)->data);

        this->kt.__fnek->Lock(fnek_id);

        QByteArray ct_encoded = ct_utf8.toBase64();

        return QString(ct_encoded);
    }

    QString dec_fn(QString enc_fn)
    {
        if(this->kt.__fnek.isNull())
        {
            throw FVFileOperationException("API misuse: should cache FNEK before computing decrypted FN");
        }

        QByteArray ct_encoded = enc_fn.toUtf8();
        QByteArray ct_utf8 = QByteArray::fromBase64(ct_encoded);
        QByteArray pt_utf8(ct_utf8.length() - crypto_secretbox_MACBYTES, 0);

        auto fnek_id = this->kt.__fnek->UnlockRO();

        if(crypto_secretbox_open_easy(
            (unsigned char*)pt_utf8.data(), (unsigned char*)ct_utf8.data(), ct_utf8.length(),
                    this->fn_revnum, this->kt.__fnek->data(fnek_id)->data
                    ) == -1)
        {
            throw FVCryptoDecryptVerifyException();
        }

        this->kt.__fnek->Lock(fnek_id);

        return QString(pt_utf8);

    }
};

struct fv_file_control_t
{
    fv_file_state_t state;
};

void FVFile::__verify(const FVUserPublicKey * owner_key)
{
    this->md->verify();
    if(owner_key != NULL)
    {
        this->md->kt.verify(*owner_key);
    }
    else
    {
        this->md->kt.verify();
    }
}


FVFile::FVFile(QFile & pt_file, FVUserKeyPair & key, const FVUserPublicKey * owner_key)
    : key(key)
{
    this->ctl = QSharedPointer<fv_file_control_t>(new fv_file_control_t);

    // Pull out some basic file info
    QFileInfo pt_fi (pt_file);
    QString pt_fn = pt_fi.fileName();
    QDir pt_dir = pt_fi.dir();

    // Verify that the file exists (a nonexistent file is bad news)
    if(!(pt_fi.exists()))
    {
        throw FVFileReadAccessException("requested plaintext file does not exist");
    }

    // Verify that this is a file and not a folder (a folder is API misuse)
    if(pt_fi.isDir())
    {
        throw FVFileOperationException("API misuse: file constructor invoked on folder");
    }

    this->filename_pt = pt_fn;
    this->path_local = pt_fi.absoluteFilePath();

    // Look for a corresponding metadata in the same directory
    QString md_fn = pt_dir.absoluteFilePath(pt_fn + FOGVAULT_FILE_MD_EXTENSION);
    QFile md_f (md_fn);
    // Does this exist?
    if(md_f.exists())
    {
        // Load the metadata from the file,
        QDataStream st(&md_f);
        this->md = QSharedPointer<fv_file_metadata_t>(new fv_file_metadata_t);
        this->md->read_from_stream(st);

        // then check signature and key table provenance
        this->__verify(owner_key);

        // verify that this is an extant file
        if(this->IsDeleted())
        {
            // TODO ???
            throw FVFileOperationException("API misuse: using plaintext constructor with 'deleted' file");
        }

        if(this->IsDirectory())
        {
            // TODO ???
            throw FVFileOperationException("API misuse: using plaintext constructor with directory");
        }

        // then load-cache the FEK and FNEK
        this->md->kt.cache_secret_keys(key);

        // and compute the ct_fn
        this->filename_enc = this->md->enc_fn(this->filename_pt);

        // has the local file size changed?
        if(this->md->pt_size != pt_fi.size())
        {
            // just update it then
            this->md->pt_size = pt_fi.size();
        }

        // mark state as pt
        this->ctl->state = FV_FILE_STATE_PT;

        // ready to go!
    }
    // Otherwise...
    else
    {
        // We need to initialize a brand new metadata entry.
        this->md = QSharedPointer<fv_file_metadata_t>(new fv_file_metadata_t);

        // set the file size
        this->md->pt_size = pt_fi.size();

        // This entry currently has no keytable entries;
        // let's add one for ourselves.
        this->md->kt.add_key(*(key.GetPubKey()), key, true);

        // FEK and FNEK are already load-cached

        // compute the ct_fn
        this->filename_enc = this->md->enc_fn(this->filename_pt);

        // mark state as pt
        this->ctl->state = FV_FILE_STATE_PT;

        // ready to go!
    }
}

FVFile::FVFile(QDir & dir, FVUserKeyPair & key, const FVUserPublicKey * owner_key)
    : key(key)
{
    this->ctl = QSharedPointer<fv_file_control_t>(new fv_file_control_t);
    // we've been given a directory so things are a little different

    // Pull out some basic info
    QString pt_fn = dir.dirName();
    QDir pt_dir = dir;
    pt_dir.cdUp();

    // Verify that the file exists (a nonexistent dir is bad news)
    if(!(dir.exists()))
    {
        throw FVFileReadAccessException("requested directory does not exist");
    }

    this->filename_pt = pt_fn;
    this->path_local = dir.absolutePath();

    // Look for a corresponding metadata in the same directory
    QString md_fn = pt_dir.absoluteFilePath(pt_fn + FOGVAULT_FILE_MD_EXTENSION);
    QFile md_f (md_fn);
    // Does this exist?
    if(md_f.exists())
    {
        // Load the metadata from the file,
        QDataStream st(&md_f);
        this->md = QSharedPointer<fv_file_metadata_t>(new fv_file_metadata_t);
        this->md->read_from_stream(st);

        // then check signature and key table provenance
        this->__verify(owner_key);

        // verify that this is an extant directory
        if(this->IsDeleted())
        {
            // TODO ???
            throw FVFileOperationException("API misuse: using directory constructor with 'deleted'");
        }

        if(!(this->IsDirectory()))
        {
            // TODO ???
            throw FVFileOperationException("API misuse: using directory constructor with file");
        }

        // then load-cache the FEK and FNEK
        this->md->kt.cache_secret_keys(key);

        // and compute the ct_fn
        this->filename_enc = this->md->enc_fn(this->filename_pt);

        // mark state as mdonly
        this->ctl->state = FV_FILE_STATE_MDONLY;

        // ready to go!
    }
    // Otherwise...
    else
    {
        // We need to initialize a brand new metadata entry.
        this->md = QSharedPointer<fv_file_metadata_t>(new fv_file_metadata_t);

        // set the file size
        this->md->pt_size = 0;

        // set the directory-ness
        this->md->is_directory |= 1;

        // This entry currently has no keytable entries;
        // let's add one for ourselves.
        this->md->kt.add_key(*(key.GetPubKey()), key, true);

        // FEK and FNEK are already load-cached

        // compute the ct_fn
        this->filename_enc = this->md->enc_fn(this->filename_pt);

        // compute the file size

        // mark state as mdonly
        this->ctl->state = FV_FILE_STATE_MDONLY;

        // ready to go!
    }
}

FVFile::FVFile(QFile & md_file, QFile & dat_file, FVUserKeyPair & key, const FVUserPublicKey * owner_key, bool encrypted)
    : key(key)
{
    this->ctl = QSharedPointer<fv_file_control_t>(new fv_file_control_t);

    // In this case we're given the FogVault metadata and PT/CT

    // Pull out some basic file info
    QFileInfo dat_fi (dat_file);
    QString dat_fn = dat_file.fileName();
    QDir dat_dir = dat_fi.dir();

    // Verify that the file exists (a nonexistent file is bad news)
    if(!(dat_file.exists()))
    {
        throw FVFileReadAccessException("requested file does not exist");
    }

    // Verify that this is a file and not a folder (a folder is API misuse)
    if(dat_fi.isDir())
    {
        throw FVFileOperationException("API misuse: file constructor invoked on folder");
    }

    this->path_local = dat_fi.absoluteFilePath();

    // Verify that the metadata file exists

    if(!(md_file.exists()))
    {
        throw FVFileReadAccessException("requested metadata file does not exist");
    }

    // and load it
    QDataStream st(&md_file);
    this->md = QSharedPointer<fv_file_metadata_t>(new fv_file_metadata_t);
    this->md->read_from_stream(st);

    // then check signature and key table provenance
    this->__verify(owner_key);

    // verify that this is an extant file
    if(this->IsDeleted())
    {
        // TODO ???
        throw FVFileOperationException("API misuse: using file+md constructor with 'deleted' file");
    }

    if(this->IsDirectory())
    {
        // TODO ???
        throw FVFileOperationException("API misuse: using file+md constructor with directory");
    }

    // load-cache the FEK and FNEK
    this->md->kt.cache_secret_keys(key);

    // compute the missing fn and set the state correctly
    if(encrypted)
    {
        this->filename_enc = dat_fn;
        this->filename_pt = this->md->dec_fn(dat_fn);
        this->ctl->state = FV_FILE_STATE_CT;
    }
    else
    {
        this->filename_pt = dat_fn;
        this->filename_enc = this->md->enc_fn(dat_fn);
        this->ctl->state = FV_FILE_STATE_PT;
    }

    // all done!
}

FVFile::FVFile(QFile & md_file, uint32_t reserved, FVUserKeyPair & key, const FVUserPublicKey * owner_key, bool encrypted)
    : key(key)
{
    this->ctl = QSharedPointer<fv_file_control_t>(new fv_file_control_t);

    /// Here we're given just the md file; there is no immediate local path.
    /// If this is a non-deleted directory then check the existence of the corresponding folder.
    ///

    // Verify that the md_file exists
    if(!(md_file.exists()))
    {
        throw FVFileReadAccessException("requested metadata file does not exist");
    }

    // and load it
    QDataStream st(&md_file);
    this->md = QSharedPointer<fv_file_metadata_t>(new fv_file_metadata_t);
    this->md->read_from_stream(st);

    // then check signature and key table provenance
    this->__verify(owner_key);

    // verify that this is a directory or deleted thing
    if(!(this->IsDirectory()) & !(this->IsDeleted()))
    {
        throw FVFileOperationException("API misuse: using md-only constructor with nondeleted nondirectory");
    }

    // load-cache the FEK and FNEK
    this->md->kt.cache_secret_keys(key);

    // compute the missing fn and set the state correctly
    QString md_fn = md_file.fileName();
    if(encrypted)
    {
        this->filename_enc = md_fn;
        this->filename_pt = this->md->dec_fn(md_fn);
    }
    else
    {
        this->filename_pt = md_fn;
        this->filename_enc = this->md->enc_fn(md_fn);
    }
    this->ctl->state = FV_FILE_STATE_MDONLY;

    // all done!
}

FVFile::~FVFile()
{
    // TODO if we need to do anything here
}

QString FVFile::PTFileName() const
{
    return filename_pt;
}

QString FVFile::CTFileName() const
{
    return filename_enc;
}

bool FVFile::IsDeleted() const
{
    return (md->is_deleted & 1);
}

bool FVFile::IsDirectory() const
{
    return (md->is_directory & 1);
}

void FVFile::SetDeleted()
{
    md->is_deleted = randombytes_random() | 1;
    this->ctl->state = FV_FILE_STATE_MDONLY;
    this->path_local.clear();
}

QString FVFile::__decrypt(QString path_to, QString path_from)
{
    QFile from(path_from);
    QFile to(path_to);

    QByteArray ct_buf(FOGVAULT_BLOCK_ENC_LENGTH, 0);
    QByteArray pt_buf(FOGVAULT_BLOCK_LENGTH, 0);

    if(!(from.open(QIODevice::ReadOnly)))
    {
        throw FVFileReadAccessException();
    }

    if(!(to.open(QIODevice::WriteOnly)))
    {
        throw FVFileWriteAccessException();
    }

    // TODO check the expected PT file size here!!

    qint64 ct_read = 0;
    quint64 pt_dec = 0;
    qint64 offset = 0;

    // HACK nonce known to be 8 bytes
    uint64_t nonce = *((uint64_t*)this->md->revnum);

    auto k_id = this->md->kt.__fek.data()->UnlockRO();
    do
    {
        QByteArray ad = this->md->adata(offset);
        ct_read = from.read(ct_buf.data(), FOGVAULT_BLOCK_ENC_LENGTH);
        if(crypto_aead_chacha20poly1305_decrypt((unsigned char*)pt_buf.data(), &pt_dec, NULL,
                                                (unsigned char*)ct_buf.data(), ct_read,
                                                (unsigned char*)ad.data(),
                                                ad.length(),
                                                this->md->revnum,
                                                this->md->kt.__fek.data()->data(k_id)->data)
                == -1)
        {
            this->md->kt.__fnek.data()->Lock(k_id);
            throw FVCryptoDecryptVerifyException();
        }
        to.write(pt_buf.data(), pt_dec);
        offset++;
        nonce++;
    } while(ct_read == FOGVAULT_BLOCK_ENC_LENGTH);

    return path_to;
}

QString FVFile::__encrypt(QString path_to, QString path_from)
{
    QFile from(path_from);
    QFile to(path_to);

    QByteArray ct_buf(FOGVAULT_BLOCK_ENC_LENGTH, 0);
    QByteArray pt_buf(FOGVAULT_BLOCK_LENGTH, 0);

    if(!(from.open(QIODevice::ReadOnly)))
    {
        throw FVFileReadAccessException();
    }

    if(!(to.open(QIODevice::WriteOnly)))
    {
        throw FVFileWriteAccessException();
    }

    // check the file size
    QFileInfo f_i(from);
    if(f_i.size() != this->md->pt_size)
    {
        throw FVFileMDSizeException();
    }

    qint64 pt_read = 0;
    quint64 ct_enc = 0;
    qint64 offset = 0;

    // HACK nonce known to be 8 bytes
    uint64_t nonce = *((uint64_t*)this->md->revnum);

    auto k_id = this->md->kt.__fek.data()->UnlockRO();
    do
    {
        QByteArray ad = this->md->adata(offset);
        pt_read = from.read(pt_buf.data(), FOGVAULT_BLOCK_LENGTH);
        crypto_aead_chacha20poly1305_encrypt((unsigned char*)ct_buf.data(),
                                             &ct_enc,
                                             (unsigned char*)pt_buf.data(),
                                             pt_read,
                                             (unsigned char*)ad.data(),
                                             ad.length(),
                                             NULL,
                                             (const unsigned char *)&nonce,
                                             this->md->kt.__fek.data()->data(k_id)->data);
        to.write(ct_buf.data(), ct_enc);
        offset++;
        nonce++;
    } while(pt_read == FOGVAULT_BLOCK_LENGTH);

    return path_to;
}

QString FVFile::__writemd(QString path_to, FVUserKeyPair & k)
{
    this->md->sign(k);

    QFile to(path_to);
    if(!(to.open(QIODevice::WriteOnly)))
    {
        throw FVFileWriteAccessException();
    }

    QDataStream st(&to);

    this->md->write_to_stream(st);

    return path_to;
}

// Write plaintext to a new file at the given directory.
QString FVFile::WritePT(QDir & dir)
{
    // does not make sense if we are a deleted file or directory
    if(this->ctl->state == FV_FILE_STATE_UNKNOWN || this->ctl->state == FV_FILE_STATE_MDONLY)
    {
        return QString();
    }
    // not necessary if we are already in plaintext mode
    if(this->ctl->state == FV_FILE_STATE_PT)
    {
        return this->path_local;
    }

    // if we are in ciphertext mode, we decrypt and write the file,
    // then switch to PT mode
    // TODO THIS IS UNSAFE: it will try to overwrite an existing file; change to write to tmp first
    this->path_local = this->__decrypt(dir.absoluteFilePath(this->filename_pt), this->path_local);
    this->ctl->state = FV_FILE_STATE_PT;

    return this->path_local;
}

QString FVFile::WritePT(const QString & path)
{
    // does not make sense if we are a deleted file or directory
    if(this->ctl->state == FV_FILE_STATE_UNKNOWN || this->ctl->state == FV_FILE_STATE_MDONLY)
    {
        return QString();
    }
    // not necessary if we are already in plaintext mode
    if(this->ctl->state == FV_FILE_STATE_PT)
    {
        return this->path_local;
    }

    // if we are in ciphertext mode, we decrypt and write the file,
    // then switch to PT mode
    this->path_local = this->__decrypt(path, this->path_local);
    this->ctl->state = FV_FILE_STATE_PT;

    return this->path_local;
}

QString FVFile::WritePT()
{
    // does not make sense if we are a deleted file or directory
    if(this->ctl->state == FV_FILE_STATE_UNKNOWN || this->ctl->state == FV_FILE_STATE_MDONLY)
    {
        return QString();
    }
    // not necessary if we are already in plaintext mode
    if(this->ctl->state == FV_FILE_STATE_PT)
    {
        return this->path_local;
    }

    // if we are in ciphertext mode, we decrypt and write the file,
    // then switch to PT mode
    QString path = QDir::temp().absoluteFilePath(this->filename_pt);
    this->path_local = this->__decrypt(path, this->path_local);
    this->ctl->state = FV_FILE_STATE_PT;

    return this->path_local;
}

QString FVFile::WriteCT()
{
    // does not make sense if we are a deleted file or directory
    if(this->ctl->state == FV_FILE_STATE_UNKNOWN || this->ctl->state == FV_FILE_STATE_MDONLY)
    {
        return QString();
    }
    // misuse of API if we are in ciphertext mode (require decrypt first)
    if(this->ctl->state == FV_FILE_STATE_CT)
    {
        throw FVFileInvalidStateException();
    }

    // if we are in plaintext mode, we encrypt and write the file to a temp directory
    QString tmp_path = QDir::temp().absoluteFilePath(this->filename_enc);
    return this->__encrypt(tmp_path, this->path_local);
}

QString FVFile::WriteMD(bool encrypt_filename)
{
    QString tmp_path = QDir::temp().absoluteFilePath((encrypt_filename ? filename_enc : filename_pt));
    return this->__writemd(tmp_path, this->key);
}

QString FVFile::WriteMD(QDir & dir, bool encrypt_filename)
{
    QString path = dir.absoluteFilePath((encrypt_filename ? filename_enc : filename_pt));
    return this->__writemd(path, this->key);
}

QVector<FVUserPublicKey> FVFile::ListFileKeys()
{
    return this->md->kt.list_keys();
}

void FVFile::AddFileKey(const FVUserPublicKey & key, FVUserKeyPair & owner_key)
{
    this->md->kt.add_key(key, owner_key);
}

void FVFile::RemoveFileKey(const FVUserPublicKey & key, FVUserKeyPair & owner_key)
{
    this->md->kt.rm_key(key, owner_key);
}

void FVFile::RemoveFileKey(unsigned int index, FVUserKeyPair & owner_key)
{
    this->md->kt.rm_key(this->md->kt.key_by_index(index), owner_key);
}
