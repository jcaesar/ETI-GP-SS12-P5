#!/bin/bash
# simple bash script to conduct the initial stuff
# I guess this is the ~fifth bash script I've written, go ahead and correct me

# if you edit any of this, make sure to clear the files first
BUILDLOG=./build.log
VALGRIND_INSTALL=./valgrind-install
VALGRIND_SRC=./valgrind-source

echo -e "Build started at $(date)" > $BUILDLOG
echo "Logging to $BUILDLOG"

START_DIR=$(pwd)
BUILDLOG=$(pwd)/$BUILDLOG
mkdir -p $VALGRIND_INSTALL
if [[ $? -ne 0 ]]; then
	echo "Could not create binary dir." | tee -a $BUILDLOG
	exit
fi
VALGRIND_INSTALL=$(cd $VALGRIND_INSTALL; pwd)
if [[ "$VALGRIND_INSTALL" == "$(pwd)" ]]; then
	echo "Invalid install dir." | tee -a $BUILDLOG
	exit
fi

if [[ -d $VALGRIND_SRC && -e $VALGRIND_SRC/memcheck/mc_main.c ]]; then
	echo "valgrind source found." | tee -a $BUILDLOG
else
	if [[ -e valgrind-3.7.0.tar.bz2 ]]; then
		echo "valgrind source package found." | tee -a $BUILDLOG
	else
		echo "Downloading valgrind source..." | tee -a $BUILDLOG
		wget http://www.valgrind.org/downloads/valgrind-3.7.0.tar.bz2 &>> $BUILDLOG
		if [[ ! -e valgrind-3.7.0.tar.bz2 ]]; then
			echo "Error at downloading" | tee -a $BUILDLOG
			exit
		fi
	fi
	echo "Unpacking valgrind..." | tee -a $BUILDLOG
	tar xjvf valgrind-3.7.0.tar.bz2 &>> $BUILDLOG
	if [[ ! -d valgrind-3.7.0 ]]; then
		echo "Error at unpacking" | tee -a $BUILDLOG
		exit
	fi
	rm valgrind-3.7.0.tar.bz2
	mv valgrind-3.7.0 $VALGRIND_SRC
fi

if [[ -x $VALGRIND_INSTALL/bin/valgrind && ( -x $VALGRIND_SRC/mctracer/mctracer-amd64-linux || -x $VALGRIND_SRC/mctracer/mctracer-x86-linux ) ]]; then
	echo "valgrind found installed, mctracer present." | tee -a $BUILDLOG
else
	START_DIR=$(pwd)
	VALGRIND_BUILD_UNCHANGED=true
	cd $VALGRIND_SRC
	if [[ -e mctracer/Makefile.am ]] && $VALGRIND_BUILD_UNCHANGED; then
		echo "patch has already been applied." | tee -a $BUILDLOG
	else
		VALGRIND_BUILD_UNCHANGED=false
		echo "patching..." | tee -a $BUILDLOG
		patch -p1 < ../mctracer-vg370.patch &>> $BUILDLOG
		if [[ ! -e mctracer/Makefile.am ]]; then
			echo "Error at patching" | tee -a $BUILDLOG
			cd $START_DIR
			exit
		fi
	fi
	if [[ -e configure ]] && $VALGRIND_BUILD_UNCHANGED; then
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
		./configure --prefix=$VALGRIND_INSTALL &>>$BUILDLOG
		if [[ ! -e Makefile ]]; then
			echo "Error at configuring" | tee -a $BUILDLOG
			cd $START_DIR
			exit
		fi
	fi
	echo -n "building valgrind... " | tee -a $BUILDLOG
	make -j 8 &>>$BUILDLOG # 8 is an a lot saner number than what plain -j does
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
	cd $START_DIR
fi
cd $START_DIR


cd simplesim
echo -n "making simplesim... "  | tee -a $BUILDLOG
make &>>$BUILDLOG
if [[ $? -eq 0 ]]; then
	echo "success";
else
	echo "failed, cleaning up..."
	make clean all &>> $BUILDLOG
fi


cd ../example
echo -n "making example... "  | tee -a $BUILDLOG
make &>>$BUILDLOG
if [[ $? -eq 0 ]]; then
	echo "success";
else
	echo "failed, cleaning up..."
	make clean all &>> $BUILDLOG
fi
cd $START_DIR


PATH_ADDITION=$VALGRIND_INSTALL/bin/:$(cd $VALGRIND_SRC; pwd)/mctracer
if [[ ! $PATH == *"$PATH_ADDITION"* ]]; then
	echo "To use valgrind, do $ export PATH=\$PATH:$PATH_ADDITION"  | tee -a $BUILDLOG
else
	echo "path is correct."
fi
