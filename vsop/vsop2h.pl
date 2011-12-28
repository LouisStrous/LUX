#!/usr/bin/perl
use strict;
use warnings;

my $desired = 1;                # 1 = A, 3 = C
$, = ', ';

my %terms;                     # @{$terms{planet}{degree}{coordinate}}

while (<>) {
  if (my @data = /^.(\d)(\d)(\d)(\d)(.{5})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{15})(.{18})(.{18})(.{14})(.{20}) $/) {
    # $data[0] = VSOP87 version (integer 1..5; 2 → VSOP B)
    # $data[1] = celestial body (integer; Mercury = 1, Neptune = 8)
    # $data[2] = coordinate index (integer ≥1)
    # $data[3] = time degree (integer; 0 → periodic, 1..5 → Poisson series)
    # $data[4] = term rank (integer)
    # $data[5..16] = coefficients of mean longitudes (integer)
    # $data[17] = amplitude S
    # $data[18] = amplitude K
    # $data[19] = amplitude A
    # $data[20] = phase B
    # $data[21] = frequency C
    next unless $data[0] == $desired;
    push @{$terms{$data[1]}{$data[2]}{$data[3]}}, map { s/\s+//; $_ } @data[19..21];
  }
}

my $letter = chr($desired + ord('A') - 1);

print <<EOD;
#include "vsop.h"

double VSOP87${letter}terms[] = {
EOD

my $i = 0;
foreach my $p (1, 2, 3, 4, 5, 6, 7, 8) {
  foreach my $c (1, 2, 3) {
    foreach my $d (0, 1, 2, 3, 4, 5) {
      my @terms = (exists $terms{$p}{$c} and exists $terms{$p}{$c}{$d})?
        @{$terms{$p}{$c}{$d}}: ();
      foreach my $term (@terms) {
        print "$term, ";
        print "\n" if $i % 3 == 2;
        $i++;
      }
    }
  }
}
print <<EOD;
};

/*
  6*3*8 elements:
  6 powers of time (0..5)
  3 coordinates (L, B, R or X, Y, Z)
  8 planets (Mercury .. Neptune)
 */

struct VSOPdata VSOP87${letter}data = {
  { /* struct planetIndex indices[6*3*8] */
EOD

my $nterms = 0;
$i = 0;
foreach my $p (1, 2, 3, 4, 5, 6, 7, 8) {
  foreach my $c (1, 2, 3) {
    foreach my $d (0, 1, 2, 3, 4, 5) {
      my $n = 0;
      $n = scalar(@{$terms{$p}{$c}{$d}})/3 if exists $terms{$p}{$c} and exists $terms{$p}{$c}{$d};
      print ' ' if $i % 4 == 0;
      print " { $nterms, $n },";
      $nterms += $n;
      print "\n" if $i % 4 == 3;
      $i++;
    }
  }
}
print <<EOD;
  },
   $nterms,  /* nTerms */
   VSOP87${letter}terms
};
EOD
