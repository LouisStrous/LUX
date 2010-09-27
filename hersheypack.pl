#! /usr/bin/perl

my $index = 0;
my @numbers;
my @indices;

print pack "L", 89869;

while (<>) {
  chomp;
  my ($number, $length, $code) = /^([ \d]{5})([ \d]{3})(.*)$/;
  push @numbers, $number;
  push @indices, $index;
  my @code = split //, $code;
  for ($i = 0; $i < $length; $i++) {
    my @coords = splice @code, 0, 2;
    @coords = map { ord($_) - ord("R") } @coords;
    if ($coords[0] == -50) {	# raise pen
      print pack "c", -50;
      $index++;
    } else {
      print pack "c2", @coords;
      $index += 2;
    }
  }
  print pack "c", 99;
  $index++;
}

print STDERR pack("L", scalar(@numbers));

for ($i = 0; $i <= $#numbers; $i++) {
  print STDERR pack("S", $numbers[$i]);
}
for ($i = 0; $i <= $#numbers; $i++) {
  print STDERR pack("L", $indices[$i]);
}


#print "x: $minx $maxx; y: $miny $maxy\n";

# max number 3926 = 0xf56, so two bytes
# max size 143, so one byte
# min x = -41
# max x = +41
# min y = -48
# max y = +41
# so we can use a signed byte for each coordinate, and can use (e.g.)
# value -50 to indicate "raise pen".
