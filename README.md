# README #

### Package Summary ###

FRANTIC (Fast Realization of ANy Templated Integrator in C++) is a modular library for building numerical integrators, with particular focus on supporting delay and stochastic differential equations.
Functions to automatically build a Qt GUI for parameter exploration based on the definition of the differential process are also included.

### Who is this for ? ###

If you need to integrate differential equations, and would like to do so in C++, this package may be of use to you. Take a look at the Delayed_Ornstein-Uhlenbeck example, specifically the ou_process.h and delayed_ou.cpp files. If you like how the definition of the differential equation is independent from the choice of integrator, then go ahead and give this library a spin.

Take care that this library currently has no active users, and is therefore unmaintained.

### Dependencies ###

This library makes liberal use of the excellent O2scl package (http://o2scl.sourceforge.net/), which itself is based on the Gnu Scientific Librairy (GSL, http://www.gnu.org/software/gsl/gsl.html). It also requires Eigen3 (https://eigen.tuxfamily.org/). Those are hard dependencies.

On Linux Eigen can usually be installed through the package manager. I have found the GSL and O2scl easiest to install by downloading .tar file for their latest release. If you try to install by cloning their git/bitbucket repositories and run into trouble, try using the .tar file instead.

For the GUI building capabilities, Qt 4 and Qwt6 are also needed.

### Getting started ###

The first step is to build O2scl according to the its instructions.

Then FRANTIC's sub-packages can be individually built (currently, frantic/integrators and frantic/ui).

Then to use in your own projects just make sure that the resulting object files and header files can be find by your linker.

### About the author ###

I (Alexandre René) originally put this library together in order to perform the numerical simulations required for my Master's studies in neurophysics.
