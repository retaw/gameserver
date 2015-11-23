	.file	"test.cpp"
	.text
	.p2align 4,,15
	.globl	_Z3funi
	.type	_Z3funi, @function
_Z3funi:
.LFB224:
	.cfi_startproc
	cvtsi2sd	%edi, %xmm0
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movsd	.LC0(%rip), %xmm1
	call	pow
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	cvttsd2si	%xmm0, %eax
	ret
	.cfi_endproc
.LFE224:
	.size	_Z3funi, .-_Z3funi
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC0:
	.long	0
	.long	1071644672
	.ident	"GCC: (GNU) 4.8.2 20140120 (Red Hat 4.8.2-16)"
	.section	.note.GNU-stack,"",@progbits
