#!/bin/bash
#
# This file is part of buteo-sync-app package
#
# Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
#
# Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA
#
#
#
#Author - Srikanth Kavoori
# This file should run unittests for sync-app
# and create the result file with unittest rate
# and coverage to this folder with name
# sync-app-results
#
# The release number should be in the file
# this script generates the results automatically 
# for the latest weekXxX directory under sync-app
#this script updation should be rarely needed 

#Script Specific Variables 

TARGET=sync-app
TARGET_WEEK=week$(date +%V)$(date +%G)
if [ $# == 0 ];
then
WD=$PWD
ROOTDIR=$WD/..
RESULTS_FILE=$TARGET-results_$TARGET_WEEK
else 
WD=$1
TARGET_WEEK=$(ls -c $WD/../$TARGET | head -1)
echo "TARGET WEEK is $TARGET_WEEK"
ROOTDIR=$WD/../$TARGET/$TARGET_WEEK
echo "ROOTDIR is $ROOTDIR"
echo $1
RESULTS_DIR=$2
echo "RESULTS_DIR is $RESULTS_DIR"
RESULTS_FILE=$RESULTS_DIR/$TARGET-results_$TARGET_WEEK
fi


echo "if running inside scratchbox use export SBOX_USE_CCACHE=no and ccache -c commands for gcov to work"
PLUGINS=(clientplugins storageplugins syncmlcommon)
PLUGINTARGETS=(syncmlclient hcalendar hcontacts hbookmarks hnotes unittest)
TEMPFILE1=$WD/.temp_results

if [ -f $TEMPFILE1 ]
then
	rm -f $TEMPFILE1
fi

TEMPFILE2=$WD/.gcov_info.txt
if [ -f $TEMPFILE2 ]
then
       rm -f $TEMPFILE2
fi

TEMPFILE3=$WD/.percent
if [ -f $TEMPFILE3 ]
then
      rm -f $TEMPFILE3
fi
echo "Running the unit tests for $TARGET..."
echo "Results will be stored in: $RESULTS_FILE ...."
cd $ROOTDIR
echo ${PLUGINS[@]}
echo ${PLUGINTARGETS[@]}
for plugin in ${PLUGINS[@]}
do
   cd $plugin
   for target in ${PLUGINTARGETS[@]}
   do
   if [ -d $target ];
   then	   
	   echo "Building the plugin $target in $PWD"
	   cd $target
#           qmake
#           make clean	   
#	   make
       if [ -d unittest ];
       then
	   cd unittest
	   fi
           echo "Building the unittest for plugin $target in $PWD"
	   if [ -f run-test ]
	   then
	      echo "Running the unittest for plugin $target in $PWD"	   
	      ./run-test 
	      cat unit_test_results.txt >> $TEMPFILE1
	      cat gcov_results.txt >> $TEMPFILE2
	      cd ..
          else    
	      echo" run-test script not present for $target "
              cd ..	      
          fi
	  cd ..
  fi
  done
if [ -d syncmlcommon ];
then
  echo "do nothing"
else
  cd ..
fi
done

# get coverage information for the files using perl
echo "executing perl $ROOTDIR/bin/gcov_info.pl $TEMPFILE2 $TEMPFILE3"
GCOV_REPORT=$WD/gcov_report.txt
perl $WD/gcov_info.pl $TEMPFILE2 $TEMPFILE3 $GCOV_REPORT

if [ ! $? -eq 0 ]; then
#echo "Perl Script for Gcov information exit normal"
#else 
 echo "Perl Script Failed to execute ... Exiting ...  "
 exit 0
fi

SUMMARY_FILE=$WD/.summary_file
if [ -f $SUMMARY_FILE ]
then
      rm -f $SUMMARY_FILE
fi
perl $WD/test_info.pl $TEMPFILE1 $SUMMARY_FILE

if [ ! $? -eq 0 ]; then
#echo "Perl Script for test information exit normal"
#else
echo "Perl Script for test information Failed to execute ... Exiting ...  "
exit 0
fi


echo "Writing the file $RESULTS_FILE"
echo "#Results for $TARGET_WEEK  " > $RESULTS_FILE          

echo "Results Summary STARTED " >> $RESULTS_FILE
                                                                                         
echo "#Current gcov reported coverage (line rate) is" >> $RESULTS_FILE
cat $TEMPFILE3 >> $RESULTS_FILE

echo "Unit test Results Summary " >> $RESULTS_FILE
cat $SUMMARY_FILE >> $RESULTS_FILE 

echo "Results Summary ENDED " >> $RESULTS_FILE

echo "****************UNIT_TEST Results **************"  >> $RESULTS_FILE
cat $TEMPFILE1 >> $RESULTS_FILE
rm -f $TEMPFILE1 $TEMPFILE3 $SUMMARY_FILE
cd $ROOTDIR
echo $PWD
qmake
make distclean
cd $WD
echo "$RESULTS_FILE created"
