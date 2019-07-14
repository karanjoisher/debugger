// NOTE(Karan): Dummy program to test the debugger
#include <stdio.h>
float i = 0.0f;
char *string = (char*)"strings!!!";
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
    fprintf(stdout, "%s\n", string);
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
 0x804846d:	db 45 f4             	fildl  -0xc(%ebp)
 0x8048470:	d9 e8                	fld1   
 0x8048472:	de c1                	faddp  %st,%st(1)
 0x8048474:	d9 1d 28 a0 04 08    	fstps  0x804a028
 0x804847a:	d9 05 28 a0 04 08    	flds   0x804a028
 0x8048480:	a1 20 a0 04 08       	mov    0x804a020,%eax
 0x8048485:	8d 64 24 f8          	lea    -0x8(%esp),%esp
 0x8048489:	dd 1c 24             	fstpl  (%esp)
 0x804848c:	68 cb 85 04 08       	push   $0x0x80485cb
 0x8048491:	50                   	push   %eax
 0x8048492:	e8 89 fe ff ff       	call   0x8048320 <fprintf@plt>
 0x8048497:	83 c4 10             	add    $0x10,%esp
 0x804849a:	83 45 f4 01          	addl   $0x1,-0xc(%ebp)
 0x804849e:	eb c7                	jmp    0x8048467 <main+0x18>
 0x80484a0:	d9 05 28 a0 04 08    	flds   0x804a028
 0x80484a6:	d9 05 28 a0 04 08    	flds   0x804a028
 0x80484ac:	d9 c9                	fxch   %st(1)
 0x80484ae:	83 ec 08             	sub    $0x8,%esp
 0x80484b1:	8d 64 24 fc          	lea    -0x4(%esp),%esp
 0x80484b5:	d9 1c 24             	fstps  (%esp)
 0x80484b8:	8d 64 24 fc          	lea    -0x4(%esp),%esp
 0x80484bc:	d9 1c 24             	fstps  (%esp)
 0x80484bf:	e8 77 ff ff ff       	call   0x804843b <_Z3Addff>
 0x80484c4:	83 c4 10             	add    $0x10,%esp
 0x80484c7:	d9 5d dc             	fstps  -0x24(%ebp)
 0x80484ca:	8b 45 dc             	mov    -0x24(%ebp),%eax
 0x80484cd:	a3 28 a0 04 08       	mov    %eax,0x804a028
 0x80484d2:	d9 05 28 a0 04 08    	flds   0x804a028
 0x80484d8:	a1 20 a0 04 08       	mov    0x804a020,%eax
 0x80484dd:	8d 64 24 f8          	lea    -0x8(%esp),%esp
 0x80484e1:	dd 1c 24             	fstpl  (%esp)
 0x80484e4:	68 cb 85 04 08       	push   $0x0x80485cb
 0x80484e9:	50                   	push   %eax
 0x80484ea:	e8 31 fe ff ff       	call   0x8048320 <fprintf@plt>
 0x80484ef:	83 c4 10             	add    $0x10,%esp
 0x80484f2:	8b 15 1c a0 04 08    	mov    0x804a01c,%edx
 0x80484f8:	a1 20 a0 04 08       	mov    0x804a020,%eax
 0x80484fd:	83 ec 04             	sub    $0x4,%esp
 0x8048500:	52                   	push   %edx
 0x8048501:	68 cf 85 04 08       	push   $0x0x80485cf
 0x8048506:	50                   	push   %eax
 0x8048507:	e8 14 fe ff ff       	call   0x8048320 <fprintf@plt>
 0x804850c:	83 c4 10             	add    $0x10,%esp
 0x804850f:	d9 05 28 a0 04 08    	flds   0x804a028
 0x8048515:	d9 7d e6             	fnstcw -0x1a(%ebp)
 0x8048518:	0f b7 45 e6          	movzwl -0x1a(%ebp),%eax
 0x804851c:	b4 0c                	mov    $0xc,%ah
 0x804851e:	66 89 45 e4          	mov    %ax,-0x1c(%ebp)
 0x8048522:	d9 6d e4             	fldcw  -0x1c(%ebp)
 0x8048525:	db 5d e0             	fistpl -0x20(%ebp)
 0x8048528:	d9 6d e6             	fldcw  -0x1a(%ebp)
 0x804852b:	8b 45 e0             	mov    -0x20(%ebp),%eax
 0x804852e:	8b 4d fc             	mov    -0x4(%ebp),%ecx
 0x8048531:	c9                   	leave  
 0x8048532:	8d 61 fc             	lea    -0x4(%ecx),%esp
 0x8048535:	c3                   	ret    
 
 
 
 -----------------------------------------------------------------------
64bit breakpoints:

  0x40058a:	55                   	push   %rbp
  0x40058b:	48 89 e5             	mov    %rsp,%rbp
  0x40058e:	48 83 ec 20          	sub    $0x20,%rsp
  0x400592:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
  0x400599:	83 7d fc 04          	cmpl   $0x4,-0x4(%rbp)
  0x40059d:	7f 48                	jg     0x4005e7 <main+0x5d>
  0x40059f:	66 0f ef c0          	pxor   %xmm0,%xmm0
  0x4005a3:	f3 0f 2a 45 fc       	cvtsi2ssl -0x4(%rbp),%xmm0
  0x4005a8:	f3 0f 10 0d 58 01 00 	movss  0x158(%rip),%xmm1        # 0x400708 <_IO_stdin_used+0x18>
  0x4005af:	00 
  0x4005b0:	f3 0f 58 c1          	addss  %xmm1,%xmm0
  0x4005b4:	f3 0f 11 05 90 0a 20 	movss  %xmm0,0x200a90(%rip)        # 60104c <i>
  0x4005bb:	00 
  0x4005bc:	f3 0f 10 05 88 0a 20 	movss  0x200a88(%rip),%xmm0        # 60104c <i>
  0x4005c3:	00 
  0x4005c4:	f3 0f 5a c0          	cvtss2sd %xmm0,%xmm0
  0x4005c8:	48 8b 05 71 0a 20 00 	mov    0x200a71(%rip),%rax        # 601040 <__TMC_END__>
  0x4005cf:	be ff 06 40 00       	mov    $0x0x4006ff,%esi
  0x4005d4:	48 89 c7             	mov    %rax,%rdi
  0x4005d7:	b8 01 00 00 00       	mov    $0x1,%eax
  0x4005dc:	e8 6f fe ff ff       	callq  0x400450 <fprintf@plt>
  0x4005e1:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
  0x4005e5:	eb b2                	jmp    0x400599 <main+0xf>
  0x4005e7:	f3 0f 10 05 5d 0a 20 	movss  0x200a5d(%rip),%xmm0        # 60104c <i>
  0x4005ee:	00 
  0x4005ef:	8b 05 57 0a 20 00    	mov    0x200a57(%rip),%eax        # 60104c <i>
  0x4005f5:	0f 28 c8             	movaps %xmm0,%xmm1
  0x4005f8:	89 45 ec             	mov    %eax,-0x14(%rbp)
  0x4005fb:	f3 0f 10 45 ec       	movss  -0x14(%rbp),%xmm0
  0x400600:	e8 61 ff ff ff       	callq  0x400566 <_Z3Addff>
  0x400605:	66 0f 7e c0          	movd   %xmm0,%eax
  0x400609:	89 05 3d 0a 20 00    	mov    %eax,0x200a3d(%rip)        # 60104c <i>
  0x40060f:	f3 0f 10 05 35 0a 20 	movss  0x200a35(%rip),%xmm0        # 60104c <i>
  0x400616:	00 
  0x400617:	f3 0f 5a c0          	cvtss2sd %xmm0,%xmm0
  0x40061b:	48 8b 05 1e 0a 20 00 	mov    0x200a1e(%rip),%rax        # 601040 <__TMC_END__>
  0x400622:	be ff 06 40 00       	mov    $0x0x4006ff,%esi
  0x400627:	48 89 c7             	mov    %rax,%rdi
  0x40062a:	b8 01 00 00 00       	mov    $0x1,%eax
  0x40062f:	e8 1c fe ff ff       	callq  0x400450 <fprintf@plt>
  0x400634:	48 8b 15 fd 09 20 00 	mov    0x2009fd(%rip),%rdx        # 601038 <string>
  0x40063b:	48 8b 05 fe 09 20 00 	mov    0x2009fe(%rip),%rax        # 601040 <__TMC_END__>
  0x400642:	be 03 07 40 00       	mov    $0x0x400703,%esi
  0x400647:	48 89 c7             	mov    %rax,%rdi
  0x40064a:	b8 00 00 00 00       	mov    $0x0,%eax
  0x40064f:	e8 fc fd ff ff       	callq  0x400450 <fprintf@plt>
  0x400654:	f3 0f 10 05 f0 09 20 	movss  0x2009f0(%rip),%xmm0        # 60104c <i>
  0x40065b:	00 
  0x40065c:	f3 0f 2c c0          	cvttss2si %xmm0,%eax
  0x400660:	c9                   	leaveq 
  0x400661:	c3                   	retq   
  
  
  
  read 0x601040 0x601080 8 2
*/