program GoodTestProgram;
	var
		x, y, z : integer
		a, b, c : char
		f : float
	;
begin
	// This line will be skipped
	write('Enter a number to count to from 0: ');
	read(x);
	write('You entered: ');
	writeln(x);

	// Test some real numbers
	f := 15.3;
	f := 3.25e-15;
	f := 19.27e+8;
	f := 55.2e10;

	z := 0;
	while (z < x) do
		begin
			write(z); write(', ');
			z := z + 1
		end;
	writeln(z);

	writeln(ord('0')); // Rest of this line skipped
	y := z * (z + (* inline comment *) ord('0') - 3) + x div 2;
	writeln(y);

	{ Muliline
	comment
	}
	if (x > y) then
		begin
			writeln('x is bigger than y!');
		end
	else
		begin
			writeln('x is smaller than y!');
		end;

	(* Another
	multline
	comment
	*)
	if (z = x) then
		begin
			writeln('z is equal to x');
		end;

	x := 65

	while (x < 90) do
		begin
			write({ another inline comment}chr(x)); write(', ');
			x := x + 1;
		end;
	writeln(chr(x));
end.