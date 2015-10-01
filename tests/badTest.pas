program BadTestProgram;
	var
		x, y, z : integer
		a, b, c : char
		f : float
	;
begin
	write('Enter a number to count to from 0: ');
	read(x);
	write('You entered: ');
	writeln(x);

	f := 3.25e-15;
	f := 16.94x;	// Real error

	x := 158j;	// Integer error
	1x := 6;	// id error

	z := 0;
	while (z < x) do
		begin
			write(z); write(', ');
			z := z + 1
		end;
	writeln(z);

	x := b;	// Type error
	x := z % 5; // Unknown character

	writeln(ord('0'));
	y := z * (z + ord('0') - 3) + x div 2;
	writeln(y);

	if (x > y) then
		begin
			writeln('x is bigger than y!');
		end
	else
		begin
			writeln('x is smaller than y!');
		end;

	if (z = x) then
		begin
			writeln('z is equal to x');
		end;

	x := 65

	while (x < 90) do
		begin
			write(chr(x)); write(', ');
			x := x + 1;
		end;
	writeln('One more to go!)	// Quote error
	writeln(chr(x));
end.