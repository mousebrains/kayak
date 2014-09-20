#include "File.H"

File::File(const std::string& filename)
  : mFilename(filename)
{
  mOkay = !stat(filename.c_str(), &mStat);
}

off_t
File::size() const
{
  return mOkay ? mStat.st_size : 0;
}

time_t
File::mtime() const
{
  return mOkay ? mStat.st_mtime : 0;
}

std::string
File::dirname(const std::string& fn)
{
  const std::string::size_type i(fn.rfind('/'));

  if (i == fn.npos)
    return "./";

  if (i != (fn.size() - 1))  // Not a trailing /
    return fn.substr(0, i + 1);

  if (fn.size() == 1) // Only a /
    return fn;

  return dirname(fn.substr(0, fn.size() - 1)); // strip off trailing / and try again
}

std::string
File::tail(const std::string& fn)
{
  const std::string::size_type i(fn.rfind('/'));

  if (i == fn.npos)
    return fn;

  if (i != (fn.size() - 1))  // Not a trailing /
    return fn.substr(i+1);

  if (fn.size() == 1) // Only a /
    return fn;

  return tail(fn.substr(0, fn.size() - 1)); // strip off trailing / and try again
}
