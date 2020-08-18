lsetup "root 6.14.04-x86_64-slc6-gcc62-opt"
lsetup cmake
SCRIPT_DIR=`cd $(dirname ${BASH_SOURCE[0]-$0}) && pwd`
alias dumpevent="$SCRIPT_DIR/extern/bin/dumpevent"
