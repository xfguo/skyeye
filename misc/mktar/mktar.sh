#!/bin/sh

# We use git archive to generate a source package from 
# git respository, but when we untar this package, it
# will untar all source files on current path without
# a separate directory. This script is to solve this 
# problem by adding a directory.

# Copy this script to the similar directory of git 
# respository and run it, it will generate a package
# on the similar directory. 

# Author: yuanzheng (tsinghuayuan86@gmail.com)


# Modify the TAR_DIR & TAR_NAME to fit your git respository name.
TAR_DIR="skyeye"
TAR_NAME="${TAR_DIR}-1-3-2.tar.gz"
FTP_PATH="lzj@192.168.0.107:/home/ftp/skyeye_1.3.2"

#Create a temporaty package.
if test -e $TAR_DIR ; then
	cd $TAR_DIR
else
	echo "Error, no directory you need!\n"
	exit
fi
git archive --format=tar HEAD | gzip > $TAR_NAME
if test -e $TAR_NAME ; then
	echo "Ok, temporaty ${TAR_NAME} have created.\n"
else
	echo "Error, create temporaty ${TAR_NAME} failed!\n"
	exit
fi

if test -e $TAR_DIR ; then
	echo "${TAR_DIR} exist. Delete it!\n"
	rm -rf ${TAR_DIR}
else
	mkdir $TAR_DIR
	echo "Create ${TAR_DIR}.\n"
fi	
tar xzvf $TAR_NAME -C $TAR_DIR

# Delete the temporaty package, prepare for the new one.
rm $TAR_NAME
if  test -e $TAR_NAME ; then
	echo "Error, have not delete ${TAR_NAME}!\n"
	exit
else
	echo "Ok, delete the temp package ${TAR_NAME}.\n"
fi

# Create the final package and delete the temp dir.
tar -zcvf ../$TAR_NAME $TAR_DIR 
rm -rf $TAR_DIR
if test -e ../$TAR_NAME ; then
	echo "Ok, new ${TAR_NAME} have created!\n"
else
	echo "Error, create ${TAR_NAME} failed!\n"
	exit
fi

# Copy the package to ftp.
#scp $TAR_NAME $FTP_PATH

exit
