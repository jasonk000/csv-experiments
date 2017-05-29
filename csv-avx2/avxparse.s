global countChars
global parseBuffer

section .data

section .text

; countChars(input, char, len)
; countChars(%rdi, %rsi, %rdx)
countChars:
    ; ensure rax is zero before starting
    xor rax, rax

    ; we are going to do 128 bytes at a time, shift left by 2^7 is same as divide by 128
    sar rdx, 7

    ; rcx is our loop counter
    mov rcx, rdx

    ; check if len <= 0
    test rcx, rcx
    jng countCharsEnd

    ; load newline to ymm0
    vpbroadcastb ymm0, [rsi] ; broadcast newline byte to ymm0

countCharsTop:

    ; load 128 bytes into ymm2,3,4,5
    vmovdqu ymm2, [rdi] ; set ymm2 = input
    vmovdqu ymm3, [rdi+32]
    vmovdqu ymm4, [rdi+64]
    vmovdqu ymm5, [rdi+96]

    ; then check if equal, by bytes, comparing ymm0 and ymmN, store in ymmN
    vpcmpeqb ymm2, ymm2, ymm0
    vpcmpeqb ymm3, ymm3, ymm0
    vpcmpeqb ymm4, ymm4, ymm0
    vpcmpeqb ymm5, ymm5, ymm0

    ; move with bitmask to 64 bit registers, upper 32 bits are zerod
    vpmovmskb r8, ymm2
    vpmovmskb r9, ymm3
    vpmovmskb r10, ymm4
    vpmovmskb r11, ymm5

    ; then use popcnt and add all the values up
    popcnt r8, r8
    popcnt r9, r9
    popcnt r10, r10
    popcnt r11, r11

    ; and add all the values
    add r8, r9
    add r10, r11
    add r8, r10
    add rax, r8

    ; increment rdi by 128 to point to next set of bytes in prep for next loop
    add rdi, 128

    sub rcx, 1
    jnz countCharsTop

countCharsEnd:

    ; all done
    ret


; parseBuffer(input, output, char1, char2, len)
; parseBuffer(%rdi, %rsi, %rdx, %rcx, %r8)
parseBuffer:

    ; load char1 to ymm0 and char2 to ymm1, do this early to free up rdx and rcx
    vpbroadcastb ymm0, [rdx] ; broadcast char1 byte to ymm0
    vpbroadcastb ymm1, [rcx] ; broadcast char2 byte to ymm0

    ; we are going to do 32 bytes at a time, shift left by 2^7 is same as divide by 128, 2^5=32, 2^6=64
    sar r8, 7

    ; rcx will be loop counter
    mov rcx, r8

    ; check if len <= 0
    test rcx, rcx
    jng parseBufferEnd

parseBufferTop:

    ; load 32 bytes into ymm2
    ; then check if equal, by bytes, comparing ymm0,1 and ymmN, store in ymmN
    ; and the results
    vmovdqu ymm2, [rdi] ; set ymm2 = input
    vmovdqu ymm4, [rdi+32]
    vmovdqu ymm6, [rdi+64]
    vmovdqu ymm8, [rdi+96]

    ; perform first 32 bits, put in ymm2
    vpcmpeqb ymm3, ymm1, ymm2
    vpcmpeqb ymm2, ymm0, ymm2
    vpor ymm2, ymm2, ymm3

    ; perform next 32 bits, put in ymm4
    vpcmpeqb ymm5, ymm1, ymm4
    vpcmpeqb ymm4, ymm0, ymm4
    vpor ymm4, ymm4, ymm5

    vpcmpeqb ymm7, ymm1, ymm6
    vpcmpeqb ymm6, ymm0, ymm6
    vpor ymm6, ymm6, ymm7
    
    vpcmpeqb ymm9, ymm1, ymm8
    vpcmpeqb ymm8, ymm0, ymm8
    vpor ymm8, ymm8, ymm9

    ; move the values into rax and rbx, note that the upper 32 bits are zero'd
    vpmovmskb rax, ymm2
    vpmovmskb rbx, ymm4

    ; and save them to the dest, but only use the low 32 bits
    mov [rsi], eax
    mov [rsi+4], ebx

    vpmovmskb rax, ymm6
    vpmovmskb rbx, ymm8
    
    mov [rsi+8], eax
    mov [rsi+12], ebx

    ; increment rdi and rsi to point to next set of bytes in prep for next loop
    add rdi, 128
    add rsi, 16

    ; and loop
    sub rcx, 1
    jnz parseBufferTop

parseBufferEnd:
    ; all done
    ret

