
.global enable_smp
enable_smp:
	mrc	p15, #0, r0, c1, c0, #1
	orr	r0, #((1<<6)|(1<<3)|(1<<0))
	mcr	p15, #0, r0, c1, c0, #1
	dsb
	isb
	bx	lr
