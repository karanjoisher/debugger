// NOTE(Karan): Dummy program to test the debugger
#include <stdio.h>
float i = 0.0f;

float Add(float a, float b)
{
    float result = a + b;
    return result;
}
int main()
{
    
    for(int x = 0; x < 5; x++)
    {
        i = (float)x + 1.0f;
        fprintf(stdout, "%f\n", i);
    }
    i = Add(i, i);
    fprintf(stdout, "%f\n", i);
    
    return (int)i;
}

/*
32bit breakpoints:

 0x804844f:	8d 4c 24 04          	lea    0x4(%esp),%ecx
 0x8048453:	83 e4 f0             	and    $0xfffffff0,%esp
 0x8048456:	ff 71 fc             	pushl  -0x4(%ecx)
 0x8048459:	55                   	push   %ebp
 0x804845a:	89 e5                	mov    %esp,%ebp
 0x804845c:	51                   	push   %ecx
 0x804845d:	83 ec 24             	sub    $0x24,%esp
 0x8048460:	c7 45 f4 00 00 00 00 	movl   $0x0,-0xc(%ebp)
 0x8048467:	83 7d f4 04          	cmpl   $0x4,-0xc(%ebp)
 0x804846b:	7f 33                	jg     0x80484a0 <main+0x51>
 breakpoint 0x804846d:	db 45 f4             	fildl  -0xc(%ebp)
 0x8048470:	d9 e8                	fld1   
 0x8048472:	de c1                	faddp  %st,%st(1)
 0x8048474:	d9 1d 24 a0 04 08    	fstps  0x804a024
 0x804847a:	d9 05 24 a0 04 08    	flds   0x804a024
 0x8048480:	a1 1c a0 04 08       	mov    0x804a01c,%eax
 0x8048485:	8d 64 24 f8          	lea    -0x8(%esp),%esp
 0x8048489:	dd 1c 24             	fstpl  (%esp)
 0x804848c:	68 a0 85 04 08       	push   $0x0x80485a0
 0x8048491:	50                   	push   %eax
 breakpoint 0x8048492:	e8 89 fe ff ff       	call   0x8048320 <fprintf@plt>
 0x8048497:	83 c4 10             	add    $0x10,%esp
 0x804849a:	83 45 f4 01          	addl   $0x1,-0xc(%ebp)
 0x804849e:	eb c7                	jmp    0x8048467 <main+0x18>
 0x80484a0:	d9 05 24 a0 04 08    	flds   0x804a024
 0x80484a6:	d9 05 24 a0 04 08    	flds   0x804a024
 0x80484ac:	d9 c9                	fxch   %st(1)
 0x80484ae:	83 ec 08             	sub    $0x8,%esp
 0x80484b1:	8d 64 24 fc          	lea    -0x4(%esp),%esp
 0x80484b5:	d9 1c 24             	fstps  (%esp)
 0x80484b8:	8d 64 24 fc          	lea    -0x4(%esp),%esp
 0x80484bc:	d9 1c 24             	fstps  (%esp)
 breakpoint 0x80484bf:	e8 77 ff ff ff       	call   0x804843b <_Z3Addff>
 0x80484c4:	83 c4 10             	add    $0x10,%esp
 0x80484c7:	d9 5d dc             	fstps  -0x24(%ebp)
 0x80484ca:	8b 45 dc             	mov    -0x24(%ebp),%eax
 0x80484cd:	a3 24 a0 04 08       	mov    %eax,0x804a024
 0x80484d2:	d9 05 24 a0 04 08    	flds   0x804a024
 0x80484d8:	a1 1c a0 04 08       	mov    0x804a01c,%eax
 0x80484dd:	8d 64 24 f8          	lea    -0x8(%esp),%esp
 0x80484e1:	dd 1c 24             	fstpl  (%esp)
 0x80484e4:	68 a0 85 04 08       	push   $0x0x80485a0
 0x80484e9:	50                   	push   %eax
 breakpoint 0x80484ea:	e8 31 fe ff ff       	call   0x8048320 <fprintf@plt>
 0x80484ef:	83 c4 10             	add    $0x10,%esp
 0x80484f2:	d9 05 24 a0 04 08    	flds   0x804a024
 0x80484f8:	d9 7d e6             	fnstcw -0x1a(%ebp)
 0x80484fb:	0f b7 45 e6          	movzwl -0x1a(%ebp),%eax
 0x80484ff:	b4 0c                	mov    $0xc,%ah
 0x8048501:	66 89 45 e4          	mov    %ax,-0x1c(%ebp)
 0x8048505:	d9 6d e4             	fldcw  -0x1c(%ebp)
 0x8048508:	db 5d e0             	fistpl -0x20(%ebp)
 0x804850b:	d9 6d e6             	fldcw  -0x1a(%ebp)
 0x804850e:	8b 45 e0             	mov    -0x20(%ebp),%eax
 0x8048511:	8b 4d fc             	mov    -0x4(%ebp),%ecx
 0x8048514:	c9                   	leave  
 0x8048515:	8d 61 fc             	lea    -0x4(%ecx),%esp
 breakpoint 0x8048518:	c3                   	ret    
 0x8048519:	66 90                	xchg   %ax,%ax
 0x804851b:	66 90                	xchg   %ax,%ax
 0x804851d:	66 90                	xchg   %ax,%ax
 0x804851f:	90                   	nop
 
 
 -----------------------------------------------------------------------
64bit breakpoints:

  0x40058a:	55                   	push   %rbp
  0x40058b:	48 89 e5             	mov    %rsp,%rbp
  0x40058e:	48 83 ec 20          	sub    $0x20,%rsp
  0x400592:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
  0x400599:	83 7d fc 04          	cmpl   $0x4,-0x4(%rbp)
  0x40059d:	7f 48                	jg     0x4005e7 <main+0x5d>
  breakpoint 0x40059f:	66 0f ef c0          	pxor   %xmm0,%xmm0
  0x4005a3:	f3 0f 2a 45 fc       	cvtsi2ssl -0x4(%rbp),%xmm0
  0x4005a8:	f3 0f 10 0d 28 01 00 	movss  0x128(%rip),%xmm1        # 0x4006d8 <_IO_stdin_used+0x8>
  0x4005af:	00 
  breakpoint 0x4005b0:	f3 0f 58 c1          	addss  %xmm1,%xmm0
  0x4005b4:	f3 0f 11 05 88 0a 20 	movss  %xmm0,0x200a88(%rip)        # 601044 <i>
  0x4005bb:	00 
  0x4005bc:	f3 0f 10 05 80 0a 20 	movss  0x200a80(%rip),%xmm0        # 601044 <i>
  0x4005c3:	00 
  0x4005c4:	f3 0f 5a c0          	cvtss2sd %xmm0,%xmm0
  0x4005c8:	48 8b 05 69 0a 20 00 	mov    0x200a69(%rip),%rax        # 601038 <__TMC_END__>
  0x4005cf:	be d4 06 40 00       	mov    $0x0x4006d4,%esi
  0x4005d4:	48 89 c7             	mov    %rax,%rdi
  0x4005d7:	b8 01 00 00 00       	mov    $0x1,%eax
  breakpoint 0x4005dc:	e8 6f fe ff ff       	callq  0x400450 <fprintf@plt>
  0x4005e1:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
  0x4005e5:	eb b2                	jmp    0x400599 <main+0xf>
  0x4005e7:	f3 0f 10 05 55 0a 20 	movss  0x200a55(%rip),%xmm0        # 601044 <i>
  0x4005ee:	00 
  0x4005ef:	8b 05 4f 0a 20 00    	mov    0x200a4f(%rip),%eax        # 601044 <i>
  0x4005f5:	0f 28 c8             	movaps %xmm0,%xmm1
  0x4005f8:	89 45 ec             	mov    %eax,-0x14(%rbp)
  0x4005fb:	f3 0f 10 45 ec       	movss  -0x14(%rbp),%xmm0
  breakpoint 0x400600:	e8 61 ff ff ff       	callq  0x400566 <_Z3Addff>
  0x400605:	66 0f 7e c0          	movd   %xmm0,%eax
  0x400609:	89 05 35 0a 20 00    	mov    %eax,0x200a35(%rip)        # 601044 <i>
  0x40060f:	f3 0f 10 05 2d 0a 20 	movss  0x200a2d(%rip),%xmm0        # 601044 <i>
  0x400616:	00 
  0x400617:	f3 0f 5a c0          	cvtss2sd %xmm0,%xmm0
  0x40061b:	48 8b 05 16 0a 20 00 	mov    0x200a16(%rip),%rax        # 601038 <__TMC_END__>
  0x400622:	be d4 06 40 00       	mov    $0x0x4006d4,%esi
  0x400627:	48 89 c7             	mov    %rax,%rdi
  0x40062a:	b8 01 00 00 00       	mov    $0x1,%eax
  breakpoint 0x40062f:	e8 1c fe ff ff       	callq  0x400450 <fprintf@plt>
  0x400634:	f3 0f 10 05 08 0a 20 	movss  0x200a08(%rip),%xmm0        # 601044 <i>
  0x40063b:	00 
  0x40063c:	f3 0f 2c c0          	cvttss2si %xmm0,%eax
  0x400640:	c9                   	leaveq 
  breakpoint 0x400641:	c3                   	retq   
  0x400642:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
  0x400649:	00 00 00 
  0x40064c:	0f 1f 40 00          	nopl   0x0(%rax)
  
*/