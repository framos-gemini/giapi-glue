#include <iostream>  
using namespace std;  
typedef int (*fptr)(int,int);  // Declaration of function pointer
int add(int x , int y)  
{  
    return x+y;  
}  
int main()  
{  
 fptr funcptr=add; // In this case we are pointing to the add function  

 int sum=funcptr(7,10);  
 std::cout << "Sum=" <<sum<< std::endl;  
  return 0;  
}  
