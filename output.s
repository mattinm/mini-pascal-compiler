.data

.text

main: addi $sp, $sp, -108
li $t0, 72
sb $t0, 0($sp)
li $t0, 111
sb $t0, 1($sp)
li $t0, 119
sb $t0, 2($sp)
li $t0, 32
sb $t0, 3($sp)
li $t0, 109
sb $t0, 4($sp)
li $t0, 97
sb $t0, 5($sp)
li $t0, 110
sb $t0, 6($sp)
li $t0, 121
sb $t0, 7($sp)
li $t0, 32
sb $t0, 8($sp)
li $t0, 70
sb $t0, 9($sp)
li $t0, 105
sb $t0, 10($sp)
li $t0, 98
sb $t0, 11($sp)
li $t0, 111
sb $t0, 12($sp)
li $t0, 110
sb $t0, 13($sp)
li $t0, 97
sb $t0, 14($sp)
li $t0, 99
sb $t0, 15($sp)
li $t0, 99
sb $t0, 16($sp)
li $t0, 105
sb $t0, 17($sp)
li $t0, 32
sb $t0, 18($sp)
li $t0, 110
sb $t0, 19($sp)
li $t0, 117
sb $t0, 20($sp)
li $t0, 109
sb $t0, 21($sp)
li $t0, 98
sb $t0, 22($sp)
li $t0, 101
sb $t0, 23($sp)
li $t0, 114
sb $t0, 24($sp)
li $t0, 115
sb $t0, 25($sp)
li $t0, 63
sb $t0, 26($sp)
li $t0, 32
sb $t0, 27($sp)
li $t0, 0
sb $t0, 28($sp)
li $t0, 73
sb $t0, 32($sp)
li $t0, 110
sb $t0, 33($sp)
li $t0, 108
sb $t0, 34($sp)
li $t0, 105
sb $t0, 35($sp)
li $t0, 110
sb $t0, 36($sp)
li $t0, 101
sb $t0, 37($sp)
li $t0, 32
sb $t0, 38($sp)
li $t0, 87
sb $t0, 39($sp)
li $t0, 104
sb $t0, 40($sp)
li $t0, 105
sb $t0, 41($sp)
li $t0, 108
sb $t0, 42($sp)
li $t0, 101
sb $t0, 43($sp)
li $t0, 45
sb $t0, 44($sp)
li $t0, 76
sb $t0, 45($sp)
li $t0, 111
sb $t0, 46($sp)
li $t0, 111
sb $t0, 47($sp)
li $t0, 112
sb $t0, 48($sp)
li $t0, 0
sb $t0, 49($sp)
li $t0, 70
sb $t0, 52($sp)
li $t0, 117
sb $t0, 53($sp)
li $t0, 110
sb $t0, 54($sp)
li $t0, 99
sb $t0, 55($sp)
li $t0, 116
sb $t0, 56($sp)
li $t0, 105
sb $t0, 57($sp)
li $t0, 111
sb $t0, 58($sp)
li $t0, 110
sb $t0, 59($sp)
li $t0, 97
sb $t0, 60($sp)
li $t0, 108
sb $t0, 61($sp)
li $t0, 32
sb $t0, 62($sp)
li $t0, 87
sb $t0, 63($sp)
li $t0, 104
sb $t0, 64($sp)
li $t0, 105
sb $t0, 65($sp)
li $t0, 108
sb $t0, 66($sp)
li $t0, 101
sb $t0, 67($sp)
li $t0, 45
sb $t0, 68($sp)
li $t0, 76
sb $t0, 69($sp)
li $t0, 111
sb $t0, 70($sp)
li $t0, 111
sb $t0, 71($sp)
li $t0, 112
sb $t0, 72($sp)
li $t0, 0
sb $t0, 73($sp)
li $t0, 82
sb $t0, 76($sp)
li $t0, 101
sb $t0, 77($sp)
li $t0, 99
sb $t0, 78($sp)
li $t0, 117
sb $t0, 79($sp)
li $t0, 114
sb $t0, 80($sp)
li $t0, 115
sb $t0, 81($sp)
li $t0, 105
sb $t0, 82($sp)
li $t0, 118
sb $t0, 83($sp)
li $t0, 101
sb $t0, 84($sp)
li $t0, 0
sb $t0, 85($sp)

sw $0, 88($sp)
sw $0, 92($sp)
sw $0, 96($sp)
sw $0, 100($sp)
sw $0, 104($sp)

j mainbody


recursivefibonacci: addi $sp, $sp, -16
sw $ra, 12($sp)
sw $a0, 0($sp)
sw $a1, 4($sp)
sw $a2, 8($sp)
j recursivefibonaccibody

recursivefibonaccibody: lw $t0, 0($sp)
li $t1, 0

bgt $t0, $t1, Lif0
b Lif0end

Lif0: li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
lw $t0, 4($sp)
li $t1, 0

beq $t0, $t1, Lif1
lw $t0, 8($sp)
li $t1, 0

beq $t0, $t1, Lif2
lw $t0, 4($sp)
lw $t1, 8($sp)
add $t0, $t0, $t1
move $a0, $t0
li $v0, 1
syscall
lw $t0, 0($sp)
li $t1, 1
sub $t0, $t0, $t1
move $a0, $t0
lw $t0, 8($sp)
move $a1, $t0
lw $t0, 4($sp)
lw $t1, 8($sp)
add $t0, $t0, $t1
move $a2, $t0
jal recursivefibonacci
b Lif2end

Lif2: li $t0, 1
move $a0, $t0
li $v0, 1
syscall
lw $t0, 0($sp)
li $t1, 1
sub $t0, $t0, $t1
move $a0, $t0
li $t0, 1
move $a1, $t0
li $t0, 1
move $a2, $t0
jal recursivefibonacci
Lif2end:

b Lif1end

Lif1: li $t0, 1
move $a0, $t0
li $v0, 1
syscall
lw $t0, 0($sp)
li $t1, 1
sub $t0, $t0, $t1
move $a0, $t0
li $t0, 1
move $a1, $t0
li $t0, 0
move $a2, $t0
jal recursivefibonacci
Lif1end:

Lif0end:

lw $ra, 12($sp)
addi $sp, $sp, 16
jr $ra


nextfibonacci: addi $sp, $sp, -12
sw $ra, 8($sp)
sw $a0, 0($sp)
sw $a1, 4($sp)
j nextfibonaccibody

nextfibonaccibody: lw $t0, 0($sp)
li $t1, 0

beq $t0, $t1, Lif3
lw $t0, 4($sp)
li $t1, 0

beq $t0, $t1, Lif4
lw $t0, 0($sp)
lw $t1, 4($sp)
add $t0, $t0, $t1
move $v0, $t0
b Lif4end

Lif4: li $t0, 1
move $v0, $t0
Lif4end:

b Lif3end

Lif3: li $t0, 1
move $v0, $t0
Lif3end:

lw $ra, 8($sp)
addi $sp, $sp, 12
jr $ra

mainbody: la $t0, 0($sp)
move $a0, $t0
li $v0, 4
syscall
li $v0, 5
syscall
sw $v0, 88($sp)
li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
li $a0, 10
li $v0, 11
syscall
la $t0, 32($sp)
move $a0, $t0
li $v0, 4
syscall
li $a0, 10
li $v0, 11
syscall
li $t0, 0
sw $t0, 92($sp)

Lwhile0: lw $t0, 92($sp)
lw $t1, 88($sp)
bge $t0, $t1, Lwhile0end
lw $t0, 92($sp)
li $t1, 1

ble $t0, $t1, Lif5
lw $t0, 104($sp)
sw $t0, 96($sp)
lw $t0, 100($sp)
lw $t1, 104($sp)
add $t0, $t0, $t1
sw $t0, 104($sp)
lw $t0, 96($sp)
sw $t0, 100($sp)
li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
lw $t0, 104($sp)
move $a0, $t0
li $v0, 1
syscall
b Lif5end

Lif5: li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
li $t0, 1
move $a0, $t0
li $v0, 1
syscall
li $t0, 1
sw $t0, 100($sp)
li $t0, 1
sw $t0, 104($sp)
Lif5end:

lw $t0, 92($sp)
li $t1, 1
add $t0, $t0, $t1
sw $t0, 92($sp)
j Lwhile0
Lwhile0end:

li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
li $a0, 10
li $v0, 11
syscall
li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
li $a0, 10
li $v0, 11
syscall
la $t0, 52($sp)
move $a0, $t0
li $v0, 4
syscall
li $a0, 10
li $v0, 11
syscall
li $t0, 0
sw $t0, 92($sp)
li $t0, 0
sw $t0, 100($sp)
li $t0, 0
sw $t0, 104($sp)

Lwhile1: lw $t0, 92($sp)
lw $t1, 88($sp)
bge $t0, $t1, Lwhile1end
lw $t0, 100($sp)
move $a0, $t0
lw $t0, 104($sp)
move $a1, $t0
jal nextfibonacci
move $t0, $v0
sw $t0, 96($sp)
li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
lw $t0, 96($sp)
move $a0, $t0
li $v0, 1
syscall
lw $t0, 104($sp)
sw $t0, 100($sp)
lw $t0, 96($sp)
sw $t0, 104($sp)
lw $t0, 92($sp)
li $t1, 1
add $t0, $t0, $t1
sw $t0, 92($sp)
j Lwhile1
Lwhile1end:

li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
li $a0, 10
li $v0, 11
syscall
li $t0, 32
li $a0, 0
move $a0, $t0
li $v0, 11
syscall
li $a0, 10
li $v0, 11
syscall
la $t0, 76($sp)
move $a0, $t0
li $v0, 4
syscall
li $a0, 10
li $v0, 11
syscall
lw $t0, 88($sp)
move $a0, $t0
li $t0, 0
move $a1, $t0
li $t0, 0
move $a2, $t0
jal recursivefibonacci
li $v0, 10
syscall