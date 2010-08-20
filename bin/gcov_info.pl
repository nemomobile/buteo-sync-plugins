#!/usr/bin/perl 
#/*
# * This file is part of buteo-sync-plugins package
# *
# * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# *
# * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
# *
# * This library is free software; you can redistribute it and/or
# * modify it under the terms of the GNU Lesser General Public License
# * version 2.1 as published by the Free Software Foundation.
# *
# * This library is distributed in the hope that it will be useful, but
# * WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# * Lesser General Public License for more details.
# *
# * You should have received a copy of the GNU Lesser General Public
# * License along with this library; if not, write to the Free Software
# * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# * 02110-1301 USA
# *
# */
# Author : Srikanth Kavoori 
# Not Tested fully yet. There could be some issues.
# If you see any issues Let me know and i shall fix it :)
open(INPUTFILE, "<$ARGV[0]");
open(EXTRACTED_INFO,">$ARGV[2]");
open(EXTRACTED_FILES,">$ARGV[3]");

$exclude_string = "/usr/include/";
$exclude_string2 = ".h";
$coverage_count_string ="Lines executed:";
$include_string = ".cpp";
$count = 0;
$filecount = 0;
$coverage_percent = 0;

while(<INPUTFILE>) {
my($line) = $_;
if ($count == 3) {
  $count = 0; 
} 
if(/$exclude_string/ || /$exclude_string2/) {
  #print "Found usr_include or header file on line $. \n" ;
   $count ++;
}

if( /$include_string/ ) {
 $count = 0;	
}	

if ($count == 0) {
  # Print the line to the extracted_info and add a newline
  print EXTRACTED_INFO "$line\n";
  if(/$coverage_count_string/) {
    $filecount ++;
    $index = rindex($line,':') + 1;
    $percent = substr($line,$index);
    $perc_index = rindex($percent,'%');
    $percent = substr($percent,0,$perc_index);
    $coverage_percent += $percent;
  }
} else {
  #count should be 1 or 2 omit the line 
  $count ++;
}
}
if($filecount != 0 ) {
    $coverage_percent=sprintf("%.2f",$coverage_percent/$filecount);
}
print "Total No. of files : $filecount Coverage Percent : $coverage_percent ";
print EXTRACTED_INFO "Total No. of files : $filecount Coverage Percent : $coverage_percent ";
open(PERCENTFILE,">$ARGV[1]");
print PERCENTFILE "$coverage_percent\n" ;
close(PERCENTFILE);
close(INPUTFILE);
close(EXTRACTED_INFO);
close(EXTRACTED_FILES);
