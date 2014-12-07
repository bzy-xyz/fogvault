/**
  @file
  @author Ben Z Yuan <bzy@mit.edu>

  @brief Test suite for the FogVault crypto.
  */

#include "crypto/CryptoCommon.hpp"
#include "crypto/UserKey.hpp"
#include "crypto/File.hpp"

#include "sodium.h"

#include <cstdio>
#include <cstdlib>

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QTextStream>


const QString key_pwd("password123456_abcdefg");
const QString key_pwd_alt("password123456_abcdeFg");
const QString data1("The quick brown fox jumps over the lazy dog. 1234567890");
const QString data1_alt("The quick brown fox jumps over the Iazy dog. 1234567890");

int main(int argc, char ** argv)
{
    QFile out;
    out.open(stdout, QIODevice::WriteOnly | QIODevice::Unbuffered);

    QTextStream outs(&out);

    // initialise FogVault
    FVCryptoInit();


    // Test 1: validate user key functionality.

    outs << "Test 1: testing user key functionality.\n\n";

    // Create a new key pair.
    FVUserKeyPair kp1;

    // Grab its public key.
    outs << "Creating a key pair.\n";
    QSharedPointer<FVUserPublicKey> kp1_pub = kp1.GetPubKey();

    // Print the public key to stdout.
    QByteArray b = kp1_pub->Serialize().toBase64();
    outs << "Checking public key display...\n";
    outs << "Public key: " << QString(b) << "\n\n";

    // Create a second key pair.
    outs << "Creating a second key pair.\n";
    FVUserKeyPair kp2;
    QSharedPointer<FVUserPublicKey> kp2_pub = kp2.GetPubKey();

    // Compute an ECDH exchange.
    outs << "Checking ECDH exchange... ";
    auto kp1_lock = kp1.SecKeyUnlock();
    auto kp2_lock = kp2.SecKeyUnlock();

    FVECDHSecretPtr s1 = kp1.ComputeECDHSharedSecret(*(kp2.GetPubKey()), kp1_lock);
    FVECDHSecretPtr s2 = kp2.ComputeECDHSharedSecret(*(kp1.GetPubKey()), kp2_lock);

    kp1.SecKeyLock(kp1_lock);
    kp2.SecKeyLock(kp2_lock);

    // Verify that the ECDH secrets are the same.

    auto s1_lock = s1->UnlockRO();
    auto s2_lock = s2->UnlockRO();

    if(sodium_memcmp(s1->const_data(s1_lock)->data, s2->const_data(s2_lock)->data, FOGVAULT_ECDH_SECRET_LENGTH) == 0)
    {
        outs << "ECDH secrets match.\n";
    }
    else
    {
        outs << "ECDH secrets DO NOT match!\n";
        out.flush();
        //exit(1);
    }
    s2->Lock(s2_lock);

    // Verify that serialization does not change public keys.
    outs << "Serializing public key 2...\n";
    QByteArray kp2_sl = kp2.GetPubKey()->Serialize();
    outs << "Unserializing public key 2... \n";
    FVUserPublicKey kp2_usl(kp2_sl);

    outs << "Checking ECDH exchange... ";
    kp1_lock = kp1.SecKeyUnlock();
    FVECDHSecretPtr s3 = kp1.ComputeECDHSharedSecret(kp2_usl, kp1_lock);
    auto s3_lock = s3->UnlockRO();

    if(sodium_memcmp(s1->const_data(s1_lock)->data, s3->const_data(s3_lock)->data, FOGVAULT_ECDH_SECRET_LENGTH) == 0)
    {
        outs << "ECDH secrets match.\n";
    }
    else
    {
        outs << "ECDH secrets DO NOT match!\n";
        out.flush();
        //exit(1);
    }
    s3->Lock(s3_lock);

    // Create a third key pair.
    outs << "Creating a third key pair.\n";
    FVUserKeyPair kp3;
    QSharedPointer<FVUserPublicKey> kp3_pub = kp3.GetPubKey();

    // Compute an ECDH exchange with 1.
    outs << "Checking ECDH exchange... ";
    kp1_lock = kp1.SecKeyUnlock();
    FVECDHSecretPtr s4 = kp1.ComputeECDHSharedSecret(*kp3_pub, kp1_lock);


    // Verify that this new shared secret is *different* from the earlier one.
    auto s4_lock = s4->UnlockRO();
    if(sodium_memcmp(s1->const_data(s1_lock)->data, s4->const_data(s4_lock)->data, FOGVAULT_ECDH_SECRET_LENGTH) != 0)
    {
        outs << "1-2 and 1-3 ECDH secrets differ.\n";
    }
    else
    {
        outs << "1-2 and 1-3 ECDH secrets DO NOT differ!\n";
        out.flush();
        //exit(1);
    }
    s4->Lock(s4_lock);

    // Sign something.

    outs << "\n";
    outs << "Signing some data...\n";
    QByteArray sig1 = kp1.ComputeSignature(data1.toUtf8(),kp1_lock);

    // Verify the signature.

    outs << "Verifying signature... ";
    try {
        kp1_pub->VerifySignature(data1.toUtf8(), sig1);
        outs << "OK\n";
    }
    catch (FVCryptoSignatureVerifyException & e)
    {
        outs << "failed!\n";
    }

    // Verify the signature against a different key.

    outs << "Verifying signature against another key... ";
    try {
        kp2_pub->VerifySignature(data1.toUtf8(), sig1);
        outs << "failed!\n";
    }
    catch (FVCryptoSignatureVerifyException & e)
    {
        outs << "OK\n";
    }

    // Verify the signature against a tampered message.

    outs << "Verifying signature against altered message... ";
    try {
        kp2_pub->VerifySignature(data1_alt.toUtf8(), sig1);
        outs << "failed!\n";
    }
    catch (FVCryptoSignatureVerifyException & e)
    {
        outs << "OK\n";
    }

    outs << "Saving key to file...\n";
    // Write a key to file.
    kp1.SaveToFile("key1.out", key_pwd);

    outs << "Loading key from file...\n";
    // Try to reload the key.
    FVUserKeyPair kp1_f("key1.out", key_pwd);

    // Verify the signature again.

    outs << "Verifying signature against reloaded key... ";
    try {
        kp1_f.GetPubKey()->VerifySignature(data1.toUtf8(), sig1);
        outs << "OK\n";
    }
    catch (FVCryptoSignatureVerifyException & e)
    {
        outs << "failed!\n";
    }

    // Try to reload the key with a bad password.
    outs << "Loading key from file (bad password)...";
    try
    {
       FVUserKeyPair kp1_g("key1.out", key_pwd_alt);
       outs << "check failed!\n";
    }
    catch (FVCryptoDecryptVerifyException & e)
    {
        outs << "check passed\n";
    }


    // Test 2: basic file library stuff.

    // Let's create a data1 file
    outs << "\n\n";
    outs << "Creating a test data file...\n";
    QFile data1_file("data1.dat");
    data1_file.open(QIODevice::WriteOnly);
    QTextStream data1_ts(&data1_file);
    data1_ts << data1;
    data1_file.close();

    // Test the FVFile unknown plaintext constructor
    outs << "Initializing a FVFile from unknown plaintext...\n";
    FVFile fvf(data1_file, kp1);

    // Check the PTFileName and CTFileName to see if they exist
    outs << "Plaintext file name: " << fvf.PTFileName() << "\n";
    outs << "Ciphertext file name: " << fvf.CTFileName() << "\n";

    // Verify properties
    outs << "File is " << (fvf.IsDeleted() ? "deleted" : "not deleted") << "\n";
    outs << "File is " << (fvf.IsDirectory() ? "a directory" : "not a directory") << "\n";

    // Time for some big cheese:
    // write the corresponding MD file
    outs << "Writing metadata file... ";
    QString fvf_md_efn = fvf.WriteMD();
    outs << "file written to " << fvf_md_efn << "\n";
    // write the corresponding CT file
    outs << "Writing ciphertext file... ";
    QString fvf_ct_fn = fvf.WriteCT();
    outs << "file written to " << fvf_ct_fn << "\n";


    // Now the real test: reloading the data1 from artifacts
    outs << "Reloading file from generated artifacts...\n";
    QFile fvf_md_efn_file(fvf_md_efn);
    QFile fvf_ct_fn_file(fvf_ct_fn);
    //fvf_md_efn_file.open(QIODevice::ReadOnly);
    //fvf_ct_fn_file.open(QIODevice::ReadOnly);
    FVFile fvf_reload(fvf_md_efn_file, fvf_ct_fn_file, kp1, NULL, true);

    // Check the PTFileName and CTFileName to see if they exist
    outs << "Plaintext file name: " << fvf_reload.PTFileName() << "\n";
    outs << "Ciphertext file name: " << fvf_reload.CTFileName() << "\n";

    // Verify properties
    outs << "File is " << (fvf_reload.IsDeleted() ? "deleted" : "not deleted") << "\n";
    outs << "File is " << (fvf_reload.IsDirectory() ? "a directory" : "not a directory") << "\n";

    // Write out plaintext :o
    outs << "Writing plaintext... ";
    QDir d(QDir::current());
    QString fvf_reload_pt_fn = fvf_reload.WritePT(d);
    outs << "file written to " << fvf_reload_pt_fn << "\n";


    // Let's try again with a REALLY BIG file

    // Let's create a data1 file
    outs << "\n\n";
    outs << "Creating a much bigger test data file...\n";
    QFile data2_file("data2.dat");
    data2_file.open(QIODevice::WriteOnly);
    QTextStream data2_ts(&data2_file);
    for(int __i = 0; __i < 1048576; __i++)
    {
        data2_ts << data1;
    }
    data2_file.close();

    // Test the FVFile unknown plaintext constructor
    outs << "Initializing a FVFile from unknown plaintext...\n";
    FVFile fvf2(data2_file, kp1);

    // Check the PTFileName and CTFileName to see if they exist
    outs << "Plaintext file name: " << fvf2.PTFileName() << "\n";
    outs << "Ciphertext file name: " << fvf2.CTFileName() << "\n";

    // Verify properties
    outs << "File is " << (fvf2.IsDeleted() ? "deleted" : "not deleted") << "\n";
    outs << "File is " << (fvf2.IsDirectory() ? "a directory" : "not a directory") << "\n";

    // Time for some big cheese:
    // write the corresponding MD file
    outs << "Writing metadata file... ";
    QString fvf_md_efn2 = fvf2.WriteMD();
    outs << "file written to " << fvf_md_efn2 << "\n";
    // write the corresponding CT file
    outs << "Writing ciphertext file... ";
    QString fvf_ct_fn2 = fvf2.WriteCT();
    outs << "file written to " << fvf_ct_fn2 << "\n";


    // Now the real test: reloading the data1 from artifacts
    outs << "Reloading file from generated artifacts...\n";
    QFile fvf_md_efn_file2(fvf_md_efn2);
    QFile fvf_ct_fn_file2(fvf_ct_fn2);
    //fvf_md_efn_file2.open(QIODevice::ReadOnly);
    //fvf_ct_fn_file2.open(QIODevice::ReadOnly);
    FVFile fvf_reload2(fvf_md_efn_file2, fvf_ct_fn_file2, kp1, NULL, true);

    // Check the PTFileName and CTFileName to see if they exist
    outs << "Plaintext file name: " << fvf_reload2.PTFileName() << "\n";
    outs << "Ciphertext file name: " << fvf_reload2.CTFileName() << "\n";

    // Verify properties
    outs << "File is " << (fvf_reload2.IsDeleted() ? "deleted" : "not deleted") << "\n";
    outs << "File is " << (fvf_reload2.IsDirectory() ? "a directory" : "not a directory") << "\n";

    // Write out plaintext :o
    outs << "Writing plaintext... ";
    QString fvf_reload_pt_fn2 = fvf_reload2.WritePT();
    outs << "file written to " << fvf_reload_pt_fn2 << "\n";


    // Now we validate the file deletion behavior

    // mark the file as deleted
    outs << "Marking file as deleted...\n";
    fvf_reload2.SetDeleted();

    // and re-export the metadata
    outs << "Exporting...\n";
    QString fvf_reload3_md_efn = fvf_reload2.WriteMD(true);

    // reload the metadata into a new file object
    outs << "Reimporting...\n";
    QFile fvf_reload3_md_file(fvf_reload3_md_efn);
    //fvf_reload3_md_file.open(QIODevice::ReadOnly);
    FVFile fvf_reload3a(fvf_reload3_md_file, 0, kp1, NULL, true);

    // and verify that it is still marked deleted and not a directory
    outs << "Reimported file is " << (fvf_reload3a.IsDeleted() ? "deleted" : "not deleted") << "\n";
    outs << "Reimported file is " << (fvf_reload3a.IsDirectory() ? "a directory" : "not a directory") << "\n";


    // Now we validate the directory creation behavior

    // let's create a directory
    QDir tmpdir(QDir::temp());
    tmpdir.mkdir("newdir");

    QDir newdir(tmpdir.absoluteFilePath("newdir"));

    outs << "\n\n";
    outs << "Creating a new FVFile for 'newdir'...\n";

    FVFile newdir_fvf(newdir, kp1);

    // Check the PTFileName and CTFileName to see if they exist
    outs << "Plaintext dir name: " << newdir_fvf.PTFileName() << "\n";
    outs << "Ciphertext dir name: " << newdir_fvf.CTFileName() << "\n";

    // Verify properties
    outs << "Dir is " << (newdir_fvf.IsDeleted() ? "deleted" : "not deleted") << "\n";
    outs << "Dir is " << (newdir_fvf.IsDirectory() ? "a directory" : "not a directory") << "\n";

    outs << "\n";

    // Output metadata
    QString newdir_md = newdir_fvf.WriteMD();
    outs << "MD written to " << newdir_md << "\n";


    outs << "\n\nAll done!\n\n";

}
