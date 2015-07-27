
%macro pushx 1-*
 %rep %0
   push %1
   %rotate 1
 %endrep
%endmacro

%macro popx 1-*
  %rep %0
    %rotate -1
    pop %1
  %endrep
%endmacro

[BITS 64]

section .text

global	entry_loader:function
global	loader_size:data
global	infos_size:data

loader_size	dq	end - entry_loader
infos_size	dq	end - info_start

entry_loader:

	pushfq
	pushx	rax, rdi, rsi, rsp, rdx, rcx

	mov	rdi, 1
	lea	rsi, [rel msg]
	mov	rdx, msg_len
	mov	rax, rdi
	syscall

	jmp	start_unpacking

msg	db	"[Unpacking...]", 10, 0
msg_len	equ	$ - msg

start_unpacking:

	mov	rax, [rel info_addr]
	mov	rcx, [rel info_size]
	mov	rdx, [rel info_key]

	add	rcx, rax

.loop	xor	byte [rax], dl
	ror	rdx, 8
	inc	rax
	cmp	rax, rcx
	jnz	.loop

	popx	rax, rdi, rsi, rsp, rdx, rcx
	popfq
	jmp	0xFFFFFFFF

info_start:
info_key:	dq	0x9999999999999999
info_addr:	dq	0xAAAAAAAAAAAAAAAA
info_size:	dq	0xBBBBBBBBBBBBBBBB

end:
