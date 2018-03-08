	.file	"simplesync.c"
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"About to increase variable %d times\n"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.string	"Done increasing variable.\n"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB2:
	.text
.LHOTB2:
	.p2align 4,,15
	.globl	increase_fn
	.type	increase_fn, @function
increase_fn:
.LFB22:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	movq	stderr(%rip), %rdi
	movl	$10000000, %edx
	movl	$.LC0, %esi
	xorl	%eax, %eax
	call	fprintf
	movl	$10000000, %eax
	.p2align 4,,10
	.p2align 3
.L2:
	lock addl	$1, (%rbx)
	subl	$1, %eax
	jne	.L2
	movq	stderr(%rip), %rcx
	movl	$26, %edx
	movl	$1, %esi
	movl	$.LC1, %edi
	call	fwrite
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE22:
	.size	increase_fn, .-increase_fn
	.section	.text.unlikely
.LCOLDE2:
	.text
.LHOTE2:
	.section	.rodata.str1.8
	.align 8
.LC3:
	.string	"About to decrease variable %d times\n"
	.section	.rodata.str1.1
.LC4:
	.string	"Done decreasing variable.\n"
	.section	.text.unlikely
.LCOLDB5:
	.text
.LHOTB5:
	.p2align 4,,15
	.globl	decrease_fn
	.type	decrease_fn, @function
decrease_fn:
.LFB23:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	movq	stderr(%rip), %rdi
	movl	$10000000, %edx
	movl	$.LC3, %esi
	xorl	%eax, %eax
	call	fprintf
	movl	$10000000, %eax
	.p2align 4,,10
	.p2align 3
.L7:
	lock subl	$1, (%rbx)
	subl	$1, %eax
	jne	.L7
	movq	stderr(%rip), %rcx
	movl	$26, %edx
	movl	$1, %esi
	movl	$.LC4, %edi
	call	fwrite
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE23:
	.size	decrease_fn, .-decrease_fn
	.section	.text.unlikely
.LCOLDE5:
	.text
.LHOTE5:
	.section	.rodata.str1.1
.LC6:
	.string	""
.LC7:
	.string	"NOT "
.LC8:
	.string	"pthread_create"
.LC9:
	.string	"pthread_join"
.LC10:
	.string	"%sOK, val = %d.\n"
	.section	.text.unlikely
.LCOLDB11:
	.section	.text.startup,"ax",@progbits
.LHOTB11:
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB24:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	xorl	%esi, %esi
	movl	$val, %ecx
	movl	$increase_fn, %edx
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	movl	$0, val(%rip)
	movq	%rsp, %rdi
	call	pthread_create
	testl	%eax, %eax
	movl	%eax, %ebx
	jne	.L24
	leaq	8(%rsp), %rdi
	xorl	%esi, %esi
	movl	$val, %ecx
	movl	$decrease_fn, %edx
	call	pthread_create
	testl	%eax, %eax
	movl	%eax, %ebx
	jne	.L24
	movq	(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join
	testl	%eax, %eax
	movl	%eax, %ebx
	jne	.L25
.L13:
	movq	8(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join
	testl	%eax, %eax
	movl	%eax, %ebx
	jne	.L26
.L14:
	movl	val(%rip), %edx
	movl	$.LC6, %esi
	movl	$.LC10, %edi
	testl	%edx, %edx
	sete	%cl
	testb	%cl, %cl
	movzbl	%cl, %ebx
	movl	$.LC7, %ecx
	cmove	%rcx, %rsi
	xorl	%eax, %eax
	call	printf
	addq	$16, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
.L25:
	.cfi_restore_state
	call	__errno_location
	movl	$.LC9, %edi
	movl	%ebx, (%rax)
	call	perror
	jmp	.L13
.L26:
	call	__errno_location
	movl	$.LC9, %edi
	movl	%ebx, (%rax)
	call	perror
	jmp	.L14
.L24:
	call	__errno_location
	movl	$.LC8, %edi
	movl	%ebx, (%rax)
	call	perror
	movl	$1, %edi
	call	exit
	.cfi_endproc
.LFE24:
	.size	main, .-main
	.section	.text.unlikely
.LCOLDE11:
	.section	.text.startup
.LHOTE11:
	.globl	val
	.bss
	.align 4
	.type	val, @object
	.size	val, 4
val:
	.zero	4
	.globl	mutex
	.align 32
	.type	mutex, @object
	.size	mutex, 40
mutex:
	.zero	40
	.ident	"GCC: (Debian 4.9.2-10) 4.9.2"
	.section	.note.GNU-stack,"",@progbits
