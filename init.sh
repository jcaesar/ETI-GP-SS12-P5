#!/bin/bash
# simple bash script to conduct the initial stuff
# I guess this is the ~fifth bash script I've written, go ahead and correct me
# if you edit any of this, make sure to clear the files first
BUILDLOG=./build.log
VALGRIND_INSTALL=valgrind-source/install

echo -e "Build started at $(date)" > $BUILDLOG
echo "Logging to $BUILDLOG"

START_DIR=$(pwd)
BUILDLOG=$(pwd)/$BUILDLOG
mkdir -p $VALGRIND_INSTALL
if [[ $? -ne 0 ]]; then
	echo "Could not create install dir." | tee -a $BUILDLOG
	exit
fi
 # absolute path hack
VALGRIND_INSTALL=$(cd $VALGRIND_INSTALL; pwd)
VALGRIND_SRC=$(cd ./valgrind-source; pwd)
if [[ "$VALGRIND_INSTALL" == "$(pwd)" ]]; then # should suffice for checking the BUILD dir
	echo "Invalid install dir." | tee -a $BUILDLOG
	exit
fi
if [[ "$VALGRIND_SRC" == "$(pwd)" ]]; then
	echo "Source dir error." | tee -a $BUILDLOG
	exit
fi

# call it with the rebuild flag to compile valgrind and mctracer as well
if [[ "$1" != "rebuild" && -x $VALGRIND_INSTALL/bin/valgrind && ( -x $VALGRIND_INSTALL/lib/valgrind/mctracer-amd64-linux || -x $VALGRIND_INSTALL/lib/valgrind/mctracer-x86-linux ) ]]; then
	echo "valgrind found installed, mctracer present." | tee -a $BUILDLOG
else
	VALGRIND_BUILD_UNCHANGED=true
	cd $VALGRIND_SRC
	if [[ -x configure ]] && $VALGRIND_BUILD_UNCHANGED; then
		echo "autogen already done." | tee -a $BUILDLOG
	else
		VALGRIND_BUILD_UNCHANGED=false
		echo "executing autogen.sh..." | tee -a $BUILDLOG
		if [[ ! -x autogen.sh ]]; then
			chmod +x autogen.sh
		fi
		./autogen.sh &>> $BUILDLOG
		if [[ ! -e configure ]]; then
			echo "Error with autogen" | tee -a $BUILDLOG
			cd $START_DIR
			exit
		fi
	fi
	if [[ -e Makefile ]] && $VALGRIND_BUILD_UNCHANGED; then
		echo "configuring already done." | tee -a $BUILDLOG
	else
		VALGRIND_BUILD_UNCHANGED=false
		echo "configuring..." | tee -a $BUILDLOG
		$VALGRIND_SRC/configure --prefix=$VALGRIND_INSTALL &>>$BUILDLOG
		if [[ ! -e Makefile ]]; then
			echo "Error at configuring" | tee -a $BUILDLOG
			cd $START_DIR
			exit
		fi
	fi
	echo -n "building valgrind... " | tee -a $BUILDLOG
	# for some weird reason, we need a double make call, at least for the first execution. explanations, anyone?
	nice -n 5 make -j 8 &>>$BUILDLOG # 8 is an a lot saner number than what plain -j does
	if [[ $? -ne 0 ]]; then
		echo "Running make a second time, first attempt is bugged sometimes." >>$BUILDLOG
		nice -n 5 make -j 8 &>>$BUILDLOG # 8 is an a lot saner number than what plain -j does
	fi
	if [[ $? -ne 0 ]]; then
		echo "failed, cleaning up and exiting" | tee -a $BUILDLOG
		make clean &>>$BUILDLOG
		cd $START_DIR
		exit
	else
		echo "success"
	fi
	echo -n "installing valgrind... "  | tee -a $BUILDLOG
	make install &>>$BUILDLOG
	if [[ $? -eq 0 ]]; then
		echo "success"
	else
		echo "failed"
	fi
fi
cd $START_DIR


cd example
echo -n "making example... "  | tee -a $BUILDLOG
make &>>$BUILDLOG
if [[ $? -eq 0 ]]; then
	echo "success";
else
	echo "failed, cleaning up..."
	make clean all &>> $BUILDLOG
fi
cd $START_DIR


cd tex
echo -n "making latex pdfs... "  | tee -a $BUILDLOG
make &>>$BUILDLOG
if [[ $? -eq 0 ]]; then
	echo "success";
else
	echo "failed, cleaning up..."
	make clean all &>> $BUILDLOG
fi
cd $START_DIR



PATH_ADDITION=$VALGRIND_INSTALL/bin
if [[ ! $PATH == *"$PATH_ADDITION"* ]]; then
	echo "To use valgrind, do $ export PATH=$PATH_ADDITION:\$PATH"  | tee -a $BUILDLOG
else
	echo "path is correct."
fi
