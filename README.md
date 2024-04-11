# chai-build
An open source c/c++ commannd line project build tool

chai currently uses timestamps of c/cpp files to determine when it needs to rebuild,
in the future this will be changed to a hash-compare algorithm, as the current implementation does 
not rebuild if include files are altered
