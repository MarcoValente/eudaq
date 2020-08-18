SCRIPT_DIR=`cd $(dirname ${BASH_SOURCE[0]-$0}) && pwd`
echo Found script dir $SCRIPT_DIR

echo Setting up environment...
source $SCRIPT_DIR/setup.zsh

echo Installing...
take build
#cmake -DUSER_ITKSTRIP_BUILD=ON -DUSER_TLU_BUILD=ON -DUSER_STCONTROL_BUILD=ON -DEUDAQ_LIBRARY_BUILD_LCIO=ON .. && make -j4 && make install -j
cmake -DUSER_ITKSTRIP_BUILD=ON -DUSER_STCONTROL_BUILD=ON -DEUDAQ_LIBRARY_BUILD_LCIO=ON .. && make -j4 && make install -j
cd -
