#ifndef IO_H
#define IO_H

#include <string>
#include <fstream>
#include <map>
#include <Eigen/Dense>

/* Some helper functions for input/output operations */

namespace cent
{
  std::ofstream getFreeFilenameToWrite(std::string filename, std::string directory="",
                                       const int max_files=100);
  /* std::map like structure for storing variables and names
   * provides a common interfaces for a programme's variables, for example for
   * a GUI wanting to modifify parameters or a function for writing/reading parameters
   */
  class VariableMap {
  public:
    addVar(std::string name, double var);
    addVar(std::string name, Eigen::Matrix var);

  private:
    std::map<std::string, double> varDoubleMap;
    std::map<std::string, Eigen::Matrix> varMatrixMap;
  };

}

#endif // IO_H
