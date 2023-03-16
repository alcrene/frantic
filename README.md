# README #

### Package Summary ###

FRANTIC (Fast Realization of ANy Templated Integrator in C++) is a library geared towards accelerating the development of simulation codes in reserch environments. Its modular design is extensible, allowing for maximum code reuse between projects. It currently includes history structure and integrator modules appropriate for delay and stochastic differential equations. The standardized communication model allows mixing and combining these various modules according to the needs of the problem of interest.
Functions to automatically build a GUI for parameter exploration based on the definition of the differential process is also included. 

### Who is this for ? ###

If you need to integrate differential equations, and would like to do so in C++, this package may be of use to you. Take a look at the Delayed_Ornstein-Uhlenbeck example, specifically the ou_process.h and delayed_ou.cpp files. If you like the way the differential equation is specified in a manner that is expressive and completely decoupled from the integrator implementation, then go ahead and give this librairy a spin.

### Dependencies ###

This library makes liberal use of the excellent O2scl package (http://o2scl.sourceforge.net/), which itself is based on the Gnu Scientific Librairy (GSL, http://www.gnu.org/software/gsl/gsl.html). It also requires Eigen3 (https://eigen.tuxfamily.org/). Those are hard dependencies.

On Linux Eigen can usually be installed through the package manager. I have found the GSL and O2scl easiest to install by downloading .tar file for their latest release. If you try to install by cloning their git/bitbucket repositories and run into trouble, try using the .tar file instead.

For the GUI building capabilities, Qt 4 and Qwt6 are also needed.

### Getting started ###

The first step is to build O2scl according to the its instructions.

Then FRANTIC's sub-packages can be individually built (currently, frantic/integrators and frantic/ui).

Then to use in your own projects just make sure that the resulting object files and header files can be find by your linker.

### About the author ###

I (Alexandre René) originally put this toolset together in order to perform the numerical simulations required for my Master's studies in neurophysics. I continue to maintain it, in the hope that it will be as useful to others as it is to me.
