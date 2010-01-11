#!/usr/bin/perl

use strict;
use warnings;

use Data::Dumper;

open my $file, '<', 'foo';

print q(type: x
count: 1000000
numbit: 3
);

while (sysread $file, my $data, 1024) {
  my @chars = unpack 'C*', $data;
  foreach my $char (@chars) {
    print "$char\n";
  }
}
