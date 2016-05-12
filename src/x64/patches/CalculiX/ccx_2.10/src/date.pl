#!/usr/bin/perl

# Script to insert the date into ccx_2.10.c
use strict;
use warnings;

my $date = localtime;

my $filename = "ccx_2.10.c";
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
