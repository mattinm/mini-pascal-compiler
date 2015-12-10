program Fibonacci;
const
	question = 'How many Fibonacci numbers? ';
	msgInline = 'Inline While-Loop';
	msgFunction = 'Functional While-Loop';
	msgRecursive = 'Recursive';

var
	n, cur, temp, f1, f2 : integer;

(* Recursive Fibonacci printing. *)
procedure recursiveFibonacci (n, f1, f2 : integer);
begin
	if n > 0 then begin
		write(' ');

		if f1 = 0 then begin
			write(1);
			recursiveFibonacci(n - 1, 1, 0);
		end
		else begin
			if f2 = 0 then begin
				write(1);
				recursiveFibonacci(n - 1, 1, 1);
			end else begin
				write(f1 + f2);
				recursiveFibonacci(n - 1, f2, f1 + f2);
			end;
		end;
	end;
end;

(* Gets the next Fibonacci value and returns it. *)
function nextFibonacci (f1, f2 : integer) : integer;
begin
	if f1 = 0 then begin
		nextFibonacci := 1;
	end
	else begin
		if f2 = 0 then begin
			nextFibonacci := 1;
		end else begin
			nextFibonacci := f1 + f2;
		end;
	end;
end;

begin
	// get user input
	write(question);
	read(n);
	writeln(' ');
	writeln(msgInline);

	// while loop test with inline computation
	cur := 0;
	while cur < n do begin
		if cur <= 1 then begin
			write(' ');
			write(1);
			f1 := 1;
			f2 := 1;
		end else begin
			// get the new value into f2 and old f2 into f1
			temp := f2;
			f2 := f1 + f2;
			f1 := temp;

			write(' ');
			write(f2);
		end;

		// increment current
		cur := cur + 1;
	end;

	writeln(' ');
	writeln(' ');
	writeln(msgFunction);

	// for-loop test with functional computation
	cur := 0;
	f1 := 0;
	f2 := 0;
	while cur < n do begin
		// calculate via our function 
		temp := nextFibonacci(f1, f2);
		write(' ');
		write(temp);

		// swap the values
		f1 := f2;
		f2 := temp;

		cur := cur + 1;
	end;

	// RECURSIVE
	writeln(' ');
	writeln(' ');
	writeln(msgRecursive);
	recursiveFibonacci(n, 0, 0);
end.