#include "io.h"

/* Return a file object for writing.
 * If the filename exists or cannot be opened, it is appended with a number and retried;
 * this process is repeated until file is successfully opened or 'max_files' is reached.
 * \todo: Add number before file extension
 * \todo: Add trailing '/' to pathname if necessary
 * \todo: Should .c_str() be used ?
 */
namespace frantic
{
  std::string get_free_filename(const std::string& directory, const std::string& filename,
                                int max_files)
  {
    std::string basename = directory + "/";  // todo: don't add if it's already there
    std::string outfilename = basename + filename;

    std::fstream outfile(outfilename, std::ios::in);

    if (outfile.is_open()) {
      // File exists; try appending numbers to find non-existing one
      for(int i=1; i<= max_files; ++i) {
        outfile.close();
        outfilename = basename + filename + "_" + std::to_string(i);
        outfile.open(outfilename, std::ios::in);
        if (!outfile.is_open()) {
          break;
        }
      }
    }

    if (!outfile.is_open()) {
      // Successfully found a free filename
      outfile.close();
//      outfile.open(outfilename.c_str(), std::ios::out);
    } else {
      outfile.close();
      outfilename = "";
    }

    return outfilename;
  }

  // \todo: Allow use of more string delim (more than one character)
  // Following two functions from http://stackoverflow.com/a/236803
  std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
      std::stringstream ss(s);
      std::string item;
      while (std::getline(ss, item, delim)) {
          elems.push_back(item);
      }
      return elems;
  }
  std::vector<std::string> split(const std::string &s, char delim) {
      std::vector<std::string> elems;
      split(s, delim, elems);
      return elems;
  }

}
