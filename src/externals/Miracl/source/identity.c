/*
 *   Program to find discrete logarithms of user identities
 *   using Pollard's rho method.
 *
 *   Suitable trap-door primes are generated by "genprime" program
 *
 *   See "Non-Interactive Public-Key Cryptography"
 *   by U. Maurer & Y. Yacobi. Proc Eurocrypt '91
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "miracl.h"
#define NPRIMES 15
#define PROOT 2

static big p,p1,order,lim1,lim2;
static big pp[NPRIMES],rem[NPRIMES];
static BOOL flag=FALSE;
static int np;

void iterate(big x,big q,big r,big a,big b)
{ /* apply Pollards random mapping */
    if (mr_compare(x,lim1)<0)
    {
        mad(x,q,q,p,p,x);
        incr(a,1,a);
        if (mr_compare(a,order)==0) zero(a);
        return;

    }
    if (mr_compare(x,lim2)<0)
    {
        mad(x,x,x,p,p,x);
        premult(a,2,a);
        if (mr_compare(a,order)>=0) subtract(a,order,a);
        premult(b,2,b);
        if (mr_compare(b,order)>=0) subtract(b,order,b);
        return;
    }
    mad(x,r,r,p,p,x);
    incr(b,1,b);
    if (mr_compare(b,order)==0) zero(b);
}

long rho(big q,big r,big m,big n)
{ /* find q^m = r^n */
    long iter,rr,i;

    char stack_mem[mr_big_reserve(6,50)];
    big ax,bx,ay,by,x,y;

    memset(stack_mem,0,mr_big_reserve(6,50));

    ax=mirvar_mem(stack_mem,0);
    bx=mirvar_mem(stack_mem,1);
    ay=mirvar_mem(stack_mem,2);
    by=mirvar_mem(stack_mem,3);
    x=mirvar_mem(stack_mem,4);
    y=mirvar_mem(stack_mem,5);

    convert(1,y);
    zero(ay); zero(by);

    iter=0L;
    rr=1L;
    do
    { /* Brent's Cycle finder */
        copy(y,x);
        copy(ay,ax);
        copy(by,bx);
        rr*=2;
        for (i=1L;i<=rr;i++)
        {
            iter++;
            iterate(y,q,r,ay,by);
            if (mr_compare(x,y)==0) break;
        }
    } while (mr_compare(x,y)!=0);
    subtract(ax,ay,m);
    if (size(m)<0) add(m,order,m);
    subtract(by,bx,n);
    if (size(n)<0) add(n,order,n);
    return iter;
}

void getprime(char *fname)
{ /* get prime details from file */
    FILE *fp;
    int i;
    fp=fopen(fname,"rt");
    if (fp==NULL)
    {
        printf("file %s not found\n",fname);
        exit(0);
    }
    fscanf(fp,"%d\n",&np);
    for (i=0;i<np;i++) cinnum(pp[i],fp);
    fclose(fp);
    convert(1,p1);
    for (i=0;i<np;i++) multiply(p1,pp[i],p1);
    incr(p1,1,p);
    if (!isprime(p))
    {
        printf("Huh - modulus is not prime!");
        exit(0);
    }
    subdiv(p,3,lim1);
    premult(lim1,2,lim2);
}

void pollard(big id,big dl)
{
    int i;
    long iter;
    big_chinese bc;
    big w,Q,R,m,n,q;
    char stack_mem[mr_big_reserve(6,50)];
    memset(stack_mem,0,mr_big_reserve(6,50));

    w=mirvar_mem(stack_mem,0);
    Q=mirvar_mem(stack_mem,1);
    R=mirvar_mem(stack_mem,2);
    m=mirvar_mem(stack_mem,3);
    n=mirvar_mem(stack_mem,4);
    q=mirvar_mem(stack_mem,5);

    copy(id,q);
    crt_init(&bc,np,pp);
    for (i=0;i<np;i++)
    { /* accumulate solutions for each pp */
        copy(p1,w);
        divide(w,pp[i],w);
        powmod(q,w,p,Q);
        powltr(PROOT,w,p,R);
        copy(pp[i],order);
        iter=rho(Q,R,m,n);
        xgcd(m,order,w,w,w);

        mad(w,n,n,order,order,rem[i]);
        printf("%9ld iterations needed\n",iter);
    }
    crt(&bc,rem,dl);  /* apply chinese remainder thereom */
    crt_end(&bc);
}

int main()
{
    int i;
    FILE *fp;
    big K,rid,id,w,a,b,n,q1;
    miracl *mip=mirsys(200,256);
    for (i=0;i<NPRIMES;i++)
    {
        pp[i]=mirvar(0);
        rem[i]=mirvar(0);
    }
    w=mirvar(0);
    n=mirvar(0);
    a=mirvar(0);
    b=mirvar(0);
    p=mirvar(0);
    p1=mirvar(0);     
    q1=mirvar(0);
    K=mirvar(0);
    lim1=mirvar(0);
    lim2=mirvar(0);
    id=mirvar(0);
    rid=mirvar(0);
    order=mirvar(0);

    printf("Enter ID= ");
    innum(rid,stdin);
    getprime((char *)"trap1.dat");
    copy(p,n);
    getprime((char *)"trap2.dat");
   
    multiply(n,p,n);
    printf("\ncomposite =\n");
    cotnum(n,stdout);
    printf("(%d bits)\n",logb2(n));
    premult(rid,256,id);   
    while (jack(id,n)!=1)
    { /* bad identity - id=256*rid+i */
        printf("No Discrete Log. for this ID -- incrementing\n");
        incr(id,1,id);
    }

    getprime((char *)"trap1.dat");
    copy(p1,q1);
    pollard(id,b);
    getprime((char *)"trap2.dat");
    pollard(id,a);

    xgcd(p1,q1,K,K,K); 
    subtract(b,a,w);
    mad(w,K,w,q1,q1,w);
    if(size(w)<0) add(w,q1,w);
    subdiv(w,2,w);
    multiply(w,p1,w);
    add(w,a,w);

    fp=fopen("secret.dat","wt");
    otnum(rid,fp);
    cotnum(w,fp);
    cotnum(n,fp);
    fclose(fp);
    printf("\nDiscrete log (secret key) \n");
    cotnum(w,stdout);
    powltr(PROOT,w,n,id);
    subdiv(id,256,id);
    otstr(id,mip->IOBUFF);
    printf("Check Identity= %s\n",mip->IOBUFF);
    return 0;
}

