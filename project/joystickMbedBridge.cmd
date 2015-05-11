@rem = '
@echo off
perl -S joystickMbedBridge.cmd %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
$, = "\n";

# The above is pretty ugly, but required for DOS/NT use.  
#
#!/usr/bin/perl
#
$usage = 
	"joystickMbedBridge.cmd fromCOMport(COM21) toCOMport(COM28)";

($#ARGV == 1) ||
	die( "$usage\n");
	
$fromPort = shift;
$toPort   = shift;
	
open( IN, "\\\\.\\".$fromPort) || 
	die( "Couldn't open port \"$fromPort\" for reading.\n");

open( OUT, ">", "\\\\.\\".$toPort) || 
	die( "Couldn't open port \"$toPort\" for writing.\n");
	
	

$a = 0;
$currentDir = 0;

close( IN);
close( OUT);

######################
# Define Subroutines #
######################

sub abs
{
	local( $a) = @_;
	
	($a < 0) &&
		return( -$a);
		
	return( $a);
}

exit( 1);
__END__

while (1)
{
	$_ = <IN>;
	(/^$/) &&
		next;
		
	($r, $theta) = split;
	($r !~ /^(\d|\d{3})+$/) &&
		next;
	($theta !~ /^\d$/) &&
		next;
	
	if (&abs( $r) != 100 && &abs( $r) != 200 &&
		($r < 0 || $r > 7))
	{
		next;
	}

	if ($theta < 0 || $theta > 7)
	{
		next;
	}
	
	print( "r, theta = ($r, $theta): ".($a++)."\n");
}

:endofperl
