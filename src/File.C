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
