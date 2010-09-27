#! /usr/bin/perl

my $minx = 0;
my $maxx = 0;
my $miny = 0;
my $maxy = 0;

while (<>) {
  chomp;
  my ($number, $length, $code) = /^([ \d]{5})([ \d]{3})(.*)$/;
  $number = $number + 0;
  print "$number:";
  my @code = split //, $code;
  for ($i = 0; $i < $length; $i++) {
    my @coords = splice @code, 0, 2;
    @coords = map { ord($_) - ord("R") } @coords;
    print " " . join(",", @coords);
  }
  print "\n";
}

#print "x: $minx $maxx; y: $miny $maxy\n";

# max size 143
# min x = -41
# max x = +41
# min y = -48
# max y = +41
