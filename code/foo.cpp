// NOTE(Karan): Dummy program to test the debugger
#include <stdio.h>
int i = 0;

int Add(int a, int b)
{
    int result = a + b;
    return result;
}
int main()
{
    
    for(int x = 0; x < 5; x++)
    {
        i = x + 1;
        fprintf(stdout, "%d\n", i);
    }
    i = Add(i, i);
    fprintf(stdout, "%d\n", i);
    
    return i;
}

/*
32bit breakpoints:
breakpoint 0x8048472 (0x0804a024a301c083)
breakpoint 0x804848f (0x10c483fffffe8ce8)
breakpoint 0x80484ad
breakpoint 0x80484cf
breakpoint 0x80484e3

0x8048451:	8d 4c 24 04          	lea    0x4(%esp),%ecx
 0x8048455:	83 e4 f0             	and    $0xfffffff0,%esp
 0x8048458:	ff 71 fc             	pushl  -0x4(%ecx)
 0x804845b:	55                   	push   %ebp
 0x804845c:	89 e5                	mov    %esp,%ebp
 0x804845e:	51                   	push   %ecx
 0x804845f:	83 ec 14             	sub    $0x14,%esp
 0x8048462:	c7 45 f4 00 00 00 00 	movl   $0x0,-0xc(%ebp)
 0x8048469:	83 7d f4 04          	cmpl   $0x4,-0xc(%ebp)
 0x804846d:	7f 2e                	jg     0x804849d <main+0x4c>
 0x804846f:	8b 45 f4             	mov    -0xc(%ebp),%eax
 0x8048472:	83 c0 01             	add    $0x1,%eax
 0x8048475:	a3 24 a0 04 08       	mov    %eax,0x804a024
 0x804847a:	8b 15 24 a0 04 08    	mov    0x804a024,%edx
 0x8048480:	a1 1c a0 04 08       	mov    0x804a01c,%eax
 0x8048485:	83 ec 04             	sub    $0x4,%esp
 0x8048488:	52                   	push   %edx
 0x8048489:	68 70 85 04 08       	push   $0x0x8048570
 0x804848e:	50                   	push   %eax
 0x804848f:	e8 8c fe ff ff       	call   0x8048320 <fprintf@plt>
 0x8048494:	83 c4 10             	add    $0x10,%esp
 0x8048497:	83 45 f4 01          	addl   $0x1,-0xc(%ebp)
 0x804849b:	eb cc                	jmp    0x8048469 <main+0x18>
 0x804849d:	8b 15 24 a0 04 08    	mov    0x804a024,%edx
 0x80484a3:	a1 24 a0 04 08       	mov    0x804a024,%eax
 0x80484a8:	83 ec 08             	sub    $0x8,%esp
 0x80484ab:	52                   	push   %edx
 0x80484ac:	50                   	push   %eax
 0x80484ad:	e8 89 ff ff ff       	call   0x804843b <_Z3Addii>
 0x80484b2:	83 c4 10             	add    $0x10,%esp
 0x80484b5:	a3 24 a0 04 08       	mov    %eax,0x804a024
 0x80484ba:	8b 15 24 a0 04 08    	mov    0x804a024,%edx
 0x80484c0:	a1 1c a0 04 08       	mov    0x804a01c,%eax
 0x80484c5:	83 ec 04             	sub    $0x4,%esp
 0x80484c8:	52                   	push   %edx
 0x80484c9:	68 70 85 04 08       	push   $0x0x8048570
 0x80484ce:	50                   	push   %eax
 0x80484cf:	e8 4c fe ff ff       	call   0x8048320 <fprintf@plt>
 0x80484d4:	83 c4 10             	add    $0x10,%esp
 0x80484d7:	a1 24 a0 04 08       	mov    0x804a024,%eax
 0x80484dc:	8b 4d fc             	mov    -0x4(%ebp),%ecx
 0x80484df:	c9                   	leave  
 0x80484e0:	8d 61 fc             	lea    -0x4(%ecx),%esp
 0x80484e3:	c3                   	ret    
 0x80484e4:	66 90                	xchg   %ax,%ax
 0x80484e6:	66 90                	xchg   %ax,%ax
 0x80484e8:	66 90                	xchg   %ax,%ax
 0x80484ea:	66 90                	xchg   %ax,%ax
 0x80484ec:	66 90                	xchg   %ax,%ax
 0x80484ee:	66 90                	xchg   %ax,%ax
 
 -----------------------------------------------------------------------
64bit breakpoints:
breakpoint 0x400598 (0x200aa3058901c083)
breakpoint 0x4005bb
breakpoint 0x4005d6
breakpoint 0x4005fb
breakpoint 0x400607

0x400580:	55                   	push   %rbp
  0x400581:	48 89 e5             	mov    %rsp,%rbp
  0x400584:	48 83 ec 10          	sub    $0x10,%rsp
  0x400588:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
  0x40058f:	83 7d fc 04          	cmpl   $0x4,-0x4(%rbp)
  0x400593:	7f 31                	jg     0x4005c6 <main+0x46>
  0x400595:	8b 45 fc             	mov    -0x4(%rbp),%eax
  0x400598:	83 c0 01             	add    $0x1,%eax
  0x40059b:	89 05 a3 0a 20 00    	mov    %eax,0x200aa3(%rip)        # 601044 <i>
  0x4005a1:	8b 15 9d 0a 20 00    	mov    0x200a9d(%rip),%edx        # 601044 <i>
  0x4005a7:	48 8b 05 8a 0a 20 00 	mov    0x200a8a(%rip),%rax        # 601038 <__TMC_END__>
  0x4005ae:	be 94 06 40 00       	mov    $0x0x400694,%esi
  0x4005b3:	48 89 c7             	mov    %rax,%rdi
  0x4005b6:	b8 00 00 00 00       	mov    $0x0,%eax
  0x4005bb:	e8 90 fe ff ff       	callq  0x400450 <fprintf@plt>
  0x4005c0:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
  0x4005c4:	eb c9                	jmp    0x40058f <main+0xf>
  0x4005c6:	8b 15 78 0a 20 00    	mov    0x200a78(%rip),%edx        # 601044 <i>
  0x4005cc:	8b 05 72 0a 20 00    	mov    0x200a72(%rip),%eax        # 601044 <i>
  0x4005d2:	89 d6                	mov    %edx,%esi
  0x4005d4:	89 c7                	mov    %eax,%edi
  0x4005d6:	e8 8b ff ff ff       	callq  0x400566 <_Z3Addii>
  0x4005db:	89 05 63 0a 20 00    	mov    %eax,0x200a63(%rip)        # 601044 <i>
  0x4005e1:	8b 15 5d 0a 20 00    	mov    0x200a5d(%rip),%edx        # 601044 <i>
  0x4005e7:	48 8b 05 4a 0a 20 00 	mov    0x200a4a(%rip),%rax        # 601038 <__TMC_END__>
  0x4005ee:	be 94 06 40 00       	mov    $0x0x400694,%esi
  0x4005f3:	48 89 c7             	mov    %rax,%rdi
  0x4005f6:	b8 00 00 00 00       	mov    $0x0,%eax
  0x4005fb:	e8 50 fe ff ff       	callq  0x400450 <fprintf@plt>
  0x400600:	8b 05 3e 0a 20 00    	mov    0x200a3e(%rip),%eax        # 601044 <i>
  0x400606:	c9                   	leaveq 
  0x400607:	c3                   	retq   
  0x400608:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
  0x40060f:	00 
*/