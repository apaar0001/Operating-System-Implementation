#include<stdio.h>
int fib(int n){
    if(n == 0 || n == 1)
    {
        return n;
    }
    else{
        return fib(n-1) + fib(n-2);
    }
}
int main(){
    int num = 0;
    // printf("Enter number : ");
    // scanf("%d", &num);
    int result = fib(40);
    printf("The result is %d\n", result);
    return 0;
}