#!/usr/bin/perl

# Script to insert the date into some source files
use strict;
use warnings;

my $date = localtime;

my $filename = "ccx_2.21.c";
open FILE, "<", $filename or die("Cannot open $filename: $!");
my @lines = <FILE>;
close(FILE);

sleep 1;

open FILE, ">", $filename or die("Cannot open $filename: $!");
foreach (@lines)
{
    s/You are using an executable made on.*/You are using an executable made on $date\\n");/g;
    print FILE;
}
close(FILE);


$filename = "ccx_2.21step.c";
open FILE, "<", $filename or die("Cannot open $filename: $!");
@lines = <FILE>;
close(FILE);

sleep 1;

open FILE, ">", $filename or die("Cannot open $filename: $!");
foreach (@lines)
{
    s/You are using an executable made on.*/You are using an executable made on $date\\n");/g;
    print FILE;
}
close(FILE);


$filename = "frd.c";
open FILE, "<", $filename or die("Cannot open $filename: $!");
@lines = <FILE>;
close(FILE);

sleep 1;

open FILE, ">", $filename or die("Cannot open $filename: $!");
foreach (@lines)
{
    s/COMPILETIME.*/COMPILETIME       $date                    \\n\",p1);/g;
    print FILE;
}
close(FILE);
