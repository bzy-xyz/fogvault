#include "fvdropbox.h"
#include <qdesktopservices.h>
#include <qtextstream.h>
FvDropbox::FvDropbox(QObject *parent) :
    QObject(parent), dropbox(APP_KEY, APP_SECRET), fvTokenFile(TOKENFILENAME)
{

}

//Returns true if it worked. Returns false on error. Returns DROPBOX_NEED_CONFIRMATION
int FvDropbox::FvDropboxTryConnect(){
    QFile fvTokenFile("FvToken");

    if (fvTokenFile.exists()) // has the application already been approved?
    {
        if(fvTokenFile.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream instream(&fvTokenFile);
            QString token = instream.readLine().trimmed();
            QString secret = instream.readLine().trimmed();
            if(!token.isEmpty() && !secret.isEmpty())
            {
                dropbox.setToken(token);
                dropbox.setTokenSecret(secret);
                fvTokenFile.close();
                return 1;
            }
        }//If it exists but can't be read, threat as if it doesn't exist
        fvTokenFile.close();
    }

    //If we can't request the Request token from dropbox, possibly because of key problems:
    if(!dropbox.requestTokenAndWait())
    {
        return false;
    }
    dropbox.setAuthMethod(QDropbox::Plaintext);
    if(!dropbox.requestAccessTokenAndWait()){  //If the account is not already connected to DB
        QDesktopServices::openUrl(dropbox.authorizeLink());
        return DROPBOX_NEED_CONFIRMATION;
    }
    else{
        return saveTokenToDisk();
    }

}

int FvDropbox::FvDropboxFinishConnecting(){

    dropbox.requestAccessTokenAndWait();

    saveTokenToDisk();
    return 1;
}
int FvDropbox::saveTokenToDisk(){
    if(!fvTokenFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
        return false;


    QTextStream saveStream(&fvTokenFile);
    saveStream << dropbox.token() << endl;
    saveStream << dropbox.tokenSecret() << endl;
    fvTokenFile.close();
    return 1;
}
