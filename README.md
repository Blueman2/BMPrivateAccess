BMPrivateAccess is a single header library of private access utilities and macros, the library requires C++20.

**Supports:**
- Private members variable (including static members)
- Member function (including static member functions) 
- Private nested type access.

**Limitations:**
- No access to private constructors and destructors
- No access to default function parameters
- No access to reference member variables

Tested on MSVC, should work on any major compiler/platform that supports C++20.

# Usage
==Access to private member variables==
```cpp
struct PrivateAccessTests  
{  
private:  
    bool bPrivateValue = false;  
    inline static int StaticValue = 0;  
};

DEFINE_PRIVATE_MEMBER_ACCESSOR(PrivateAccessTests, bPrivateValue, bool)  
DEFINE_PRIVATE_MEMBER_ACCESSOR(PrivateAccessTests, StaticValue, int)

int main()
{
	PrivateAccessTests PrivateTests{};
	
	bool& ValueRef = PrivateAccessTests_Private::Get_bPrivateValue(PrivateTests);  
	int& IntValueRef = PrivateAccessTests_Private::Get_StaticValue();
}
```

==Access to Private member functions==
```cpp
struct PrivateAccessTests  
{  
private:  
    void SomeFunction()  
    {  
    }  
    
    bool GetPrivateValue() const  
    {  
        return bPrivateValue;  
    }
    
    static void SomeStaticFunction()  
    {
    
    }  
    
    bool bPrivateValue = false;
};  
  
DEFINE_PRIVATE_FUNCTION_ACCESSOR(PrivateAccessTests, SomeFunction, void)  
DEFINE_PRIVATE_FUNCTION_ACCESSOR(PrivateAccessTests, GetPrivateValue, bool)  
DEFINE_PRIVATE_FUNCTION_ACCESSOR(PrivateAccessTests, SomeStaticFunction, void)  
  
int main(int argc, char* argv[])  
{  
    PrivateAccessTests PrivateTests{};  
  
    PrivateAccessTests_Private::Call_SomeFunction(PrivateTests);  
    bool bResult = PrivateAccessTests_Private::Call_GetPrivateValue(PrivateTests); 
    PrivateAccessTests_Private::Call_SomeStaticFunction();
}
```

==Access to private nested types==
```cpp
  
struct PrivateAccessTests  
{  
private:  
    struct NestedPrivateType  
    {  
        int Value = 0;  
    };    bool GetPrivateValue()  
};  
  
DEFINE_PRIVATE_TYPE_ACCESSOR(PrivateAccessTests, NestedPrivateType)  
  
int main(int argc, char* argv[])  
{  
    PrivateAccessTests_NestedPrivateType_Type NestedPrivateType{};  
    NestedPrivateType.Value = 10;
}
```

#### Calling a "super" function from a derived class
**The library provides a dangerous way of calling a "super" function from a function override in a derived class by swapping the vtable ptr in combination with private access workarounds.**

I would **not** recommend using this for anything more than testing for these reasons:
```
* DANGER: The function swaps the vtable of the provided object with the vtable of the parent object for the duration of the callable  
* - This is UB and only works on compilers that place the vtable pointer at the start of the object  
* - During the call to any parent functions, the object will be treated as the parent object  
* - Executing any virtual functions on the object will call the parent function  
* - Calling any virtual function not present in the parent object will result in a crash or worse
```

But... if you must proceed
```cpp
struct CallSuperTestBase  
{  
    virtual ~CallSuperTestBase() = default;  
  
private:  
    virtual void Test()  
    {
    }
};  
  
DEFINE_PRIVATE_FUNCTION_ACCESSOR(CallSuperTestBase, Test, void)  
  
struct CallSuperTest : public CallSuperTestBase  
{  
    virtual void Test() override  
    {  
        BMPrivateAccess::Danger::SwapVTable(*this, CallSuperTestBase{}, [&]()  
        {  
           CallSuperTestBase_Private::Call_Test(*this);  
        });
    }};
```