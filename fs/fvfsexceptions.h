#ifndef FVFSEXCEPTIONS_H
#define FVFSEXCEPTIONS_H
#include <exception>
#include <stdexcept>

class FvFsExceptionBase : public std::runtime_error
{
public:
    FvFsExceptionBase(const std::string & extra, const std::string & desc = "unknown FogVault FileSystem exception")
        : std::runtime_error(desc + ": " + extra)
    {

    }
};

class FvFsDropboxRequestTokenException : public FvFsExceptionBase
{
public:
    FvFsDropboxRequestTokenException(const std::string & extra ="Error requesting token", const std::string & desc = "Fvdropbox request token exception")
        : FvFsExceptionBase(extra, desc)
    {

    }
};

class FvFsDropboxFileException : public FvFsExceptionBase
{
public:
      FvFsDropboxFileException(const std::string & extra ="Error operating on file", const std::string & desc = "Fvdropbox file exception")
        : FvFsExceptionBase(extra, desc)
    {

    }
};

class FvFsDropboxPathException : public FvFsExceptionBase
{
public:
      FvFsDropboxPathException(const std::string & extra ="Error converting path", const std::string & desc = "Fvdropbox path exception")
        : FvFsExceptionBase(extra, desc)
    {

    }
};


class FvFsFilePathException : public FvFsExceptionBase
{
public:
      FvFsFilePathException(const std::string & extra ="Error converting path", const std::string & desc = "FvFs path exception")
        : FvFsExceptionBase(extra, desc)
    {

    }
};



#endif // FVFSEXCEPTIONS_H
