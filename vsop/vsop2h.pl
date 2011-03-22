#!/usr/bin/perl
use strict;
use warnings;

open DATA, '<', '/home/strous/src/ana/vsop.h' or die "Cannot open vsop.h";
my $accept = 0;
my @data;
while (<DATA>) {
  chomp;
  if ($accept) {
    $accept = 0, next if /\}/;
    my @values = split /, */;
    next unless @values;
    pop @values if $values[$#values] =~ /^\s*$/; # remove final element
    push @data, @values;
  } else {
    $accept = 1 if /planetTermsD/;
  }
}
close DATA;

my $desired = 4;                # VSOP D
$, = ', ';

my %terms;                     # @{$terms{planet}{degree}{coordinate}}

while (<>) {
  if (my @data = /^.(\d)(\d)(\d)(\d)(.{5})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{3})(.{15})(.{18})(.{18})(.{14})(.{20}) $/) {
    # $data[0] = VSOP87 version (integer 0..5; 2 → VSOP B)
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

print <<EOD;
/*
  6*3*8 elements:
  6 powers of time (0..5)
  3 coordinates (L, B, R)
  8 planets (Mercury .. Neptune)
 */

struct planetIndex { short int index; short int nTerms; }
planetIndices[6*3*8] = {
EOD

my $nterms = 0;
my $i = 0;
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
};

struct planetIndex
planetIndicesTrunc[6*3*8] = {
EOD

my $limit = 1e-6;
$nterms = 0;
$i = 0;
foreach my $p (1, 2, 3, 4, 5, 6, 7, 8) {
  foreach my $c (1, 2, 3) {
    foreach my $d (0, 1, 2, 3, 4, 5) {
      my $n = 0;
      my @terms;
      @terms = @{$terms{$p}{$c}{$d}} if exists $terms{$p}{$c} and exists $terms{$p}{$c}{$d};
      $n = @terms/3;
      my $small = 0;
      for ($small = 0; $small < $n; $small++) {
        last if abs($terms[3*$small]) < $limit;
      }
      print ' ' if $i % 4 == 0;
      print " { $nterms, $small },";
      $nterms += $n;
      print "\n" if $i % 4 == 3;
      $i++;
    }
  }
}

print <<EOD;
};

/*
  3*$nterms
  3 coefficients (amplitude, phase at time zero, phase rate)
  $nterms terms
 */

double planetTerms[3*$nterms] = {
EOD

$i = 0;
foreach my $p (sort keys %terms) {
  foreach my $c (sort keys %{$terms{$p}}) {
    foreach my $d (sort keys %{$terms{$p}{$c}}) {
      my @terms = @{$terms{$p}{$c}{$d}};
      foreach my $term (@terms) {
        print "$term, ";
        print "\n" if $i % 18 == 17;
        $i++;
      }
    }
  }
}
print <<EOD;

};
EOD
