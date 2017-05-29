global parseBuffer

section .data
section .text

; parseBuffer(input, output, char1, char2, char3, len)
; parseBuffer(%rdi, %rsi, %rdx, %rcx, %r8, %r9)
parseBuffer:

    ; load char1 to ymm0 and char2 to ymm1, do this early to free up rdx and rcx
    vpbroadcastb ymm0, [rdx] ; broadcast char1 byte to ymm0
    vpbroadcastb ymm1, [rcx] ; broadcast char2 byte to ymm1
    vpbroadcastb ymm2, [r8]  ; broadcast char3 byte to ymm2

    ; we are going to do 128 bytes at a time, shift left by 2^7 is same as divide by 128, 2^5=32, 2^6=64
    sar r9, 7

    ; rcx will be loop counter
    mov rcx, r9

    ; check if len == 0
    test rcx, rcx
    jng parseBufferEnd

parseBufferTop:

    ; load 32 bytes into ymm2
    ; then check if equal, by bytes, comparing ymm0,1 and ymmN, store in ymmN
    ; and the results
    vmovdqu ymm3, [rdi] ; set ymm3 = input
    vmovdqu ymm4, [rdi+32]
    vmovdqu ymm5, [rdi+64]
    vmovdqu ymm6, [rdi+96]

    ; check against char1
    vpcmpeqb ymm7, ymm0, ymm3
    vpcmpeqb ymm8, ymm0, ymm4
    vpcmpeqb ymm9, ymm0, ymm5
    vpcmpeqb ymm10, ymm0, ymm6

    ; check against char2
    vpcmpeqb ymm11, ymm1, ymm3
    vpcmpeqb ymm12, ymm1, ymm4
    vpcmpeqb ymm13, ymm1, ymm5
    vpcmpeqb ymm14, ymm1, ymm6

    ; compact char1 and char2 results
    vpor ymm7, ymm7, ymm11
    vpor ymm8, ymm8, ymm12
    vpor ymm9, ymm9, ymm13
    vpor ymm10, ymm10, ymm14

    ; check against char3
    vpcmpeqb ymm11, ymm2, ymm3
    vpcmpeqb ymm12, ymm2, ymm4
    vpcmpeqb ymm13, ymm2, ymm5
    vpcmpeqb ymm14, ymm2, ymm6

    ; combine char1|2 and char3
    vpor ymm7, ymm7, ymm11
    vpor ymm8, ymm8, ymm12
    vpor ymm9, ymm9, ymm13
    vpor ymm10, ymm10, ymm14

    ; move the values into rax and rbx, note that the upper 32 bits are zero'd
    vpmovmskb rax, ymm7
    vpmovmskb rbx, ymm8

    ; and save them to the dest, but only use the low 32 bits
    mov [rsi], eax
    mov [rsi+4], ebx

    vpmovmskb rax, ymm9
    vpmovmskb rbx, ymm10
    
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
    vzeroupper
    ret

