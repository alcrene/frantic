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
  std::string get_free_filename(std::string directory, std::string filename,
                                const int max_files)
  {
    directory = directory + "/";
    std::string outfilename = directory + filename;

    std::fstream outfile(outfilename, std::ios::in);

    if (outfile.is_open()) {
      // File exists; try appending numbers to find non-existing one
      for(int i=1; i<= max_files; ++i) {
        outfile.close();
        outfilename = directory + filename + "_" + std::to_string(i);
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

}
