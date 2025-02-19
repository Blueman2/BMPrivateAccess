/*
* MIT License

Copyright (c) 2025 BlueMan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <type_traits>

#define DEFINE_PRIVATE_MEMBER_ACCESSOR(ClassName, MemberName, Type, ...) \
namespace BMPrivateAccess\
{\
    struct ClassName##MemberName##Tag##__VA_OPT__(T##__VA_ARGS__)\
    {\
        template<typename... TArgs>\
        friend auto& AccessPrivate(BMPrivateAccess::ClassName##MemberName##Tag##__VA_OPT__(T##__VA_ARGS__), TArgs&&... Args);\
    };\
    template struct TAccessPrivateMember<ClassName##MemberName##Tag##__VA_OPT__(T##__VA_ARGS__), &ClassName##__VA_OPT__(<__VA_ARGS__>)::MemberName>;\
}\
namespace ClassName##__VA_OPT__(_T##__VA_ARGS__)##_Private\
{\
    template<typename... TArgs>\
    static auto& Get_##MemberName(TArgs&&... Args)\
    {\
        return AccessPrivate(BMPrivateAccess::ClassName##MemberName##Tag##__VA_OPT__(T##__VA_ARGS__){}, std::forward<TArgs>(Args)...);\
    }\
}\

#define PRIVATE_ACCESS_PARAN_LEFT (
#define PRIVATE_ACCESS_PARAN_RIGHT )

#define PRIVATE_CAT_PASTE(a, b) a##b
#define PRIVATE_CAT(a, b) PRIVATE_CAT_PASTE(a, b)

#define PRIVATE_IMPL_VALUE_IF1(True, False) True
#define PRIVATE_IMPL_VALUE_IF0(True, False) False
#define PRIVATE_IMPL_VALUE_IF(True, False) False
#define PRIVATE_VALUE_IF(condition, ...) PRIVATE_IMPL_VALUE_IF##condition(__VA_ARGS__)

#define CREATE_OVERLOAD_HELPER(ClassName, MemberName, ReturnType, ...)\
    auto* PRIVATE_CAT(_GetType##ClassName##MemberName,__LINE__)(ReturnType(ClassName::*)(__VA_ARGS__))\
    {\
        using TFunctionPtr = ReturnType(ClassName::*)(__VA_ARGS__);\
        TFunctionPtr* FunctionPtr = nullptr;\
        return FunctionPtr;\
    }\
\
    auto* PRIVATE_CAT(_GetType##ClassName##MemberName,__LINE__)(ReturnType(ClassName::*)(__VA_ARGS__) const)\
    {\
        using TFunctionPtr = ReturnType(ClassName::*)(__VA_ARGS__) const;\
        TFunctionPtr* FunctionPtr = nullptr;\
        return FunctionPtr;\
    }\

#define GET_FUNCTION_TAG_TYPE(ClassName, MemberName, ...) ClassName##MemberName##Tag##__VA_OPT__(<__VA_ARGS__>)
#define CREATE_CALL_FUNCTION(ClassName, MemberName, ReturnType, ...)\
    template<typename... TArgs>\
    static ReturnType Call_##MemberName(TArgs&&... Args)\
    {\
        return CallPrivate(BMPrivateAccess::GET_FUNCTION_TAG_TYPE(ClassName, MemberName, __VA_ARGS__){}, std::forward<TArgs>(Args)...);\
    }\

#define CREATE_CALL_FUNCTION_OVERLOAD(ClassName, MemberName, ReturnType, ...)\
    template<typename... TArgs>\
    requires(BMPrivateAccess::TOverloadHelper<TArgs...>::template bSame<__VA_ARGS__>)\
    static ReturnType Call_##MemberName(ClassName& Obj, TArgs&&... Args)\
    {\
        return CallPrivate(BMPrivateAccess::GET_FUNCTION_TAG_TYPE(ClassName, MemberName, __VA_ARGS__){}, Obj, std::forward<TArgs>(Args)...);\
    }\
\
    template<typename... TArgs>\
    requires(BMPrivateAccess::TOverloadHelper<TArgs...>::template bSame<__VA_ARGS__>)\
    static ReturnType Call_##MemberName(const ClassName& Obj, TArgs&&... Args)\
    {\
        return CallPrivate(BMPrivateAccess::GET_FUNCTION_TAG_TYPE(ClassName, MemberName, __VA_ARGS__){}, Obj, std::forward<TArgs>(Args)...);\
    }\
    

#define DEFINE_PRIVATE_FUNCTION_ACCESSOR(ClassName, MemberName, ReturnType, ...) \
namespace BMPrivateAccess\
{\
    __VA_OPT__(template<typename... TArgs>)\
    __VA_OPT__(struct ClassName##MemberName##Tag;)\
\
    __VA_OPT__(template<>)\
    struct GET_FUNCTION_TAG_TYPE(ClassName, MemberName, __VA_ARGS__)\
    {\
        template<typename... TArgs>\
        friend ReturnType CallPrivate(GET_FUNCTION_TAG_TYPE(ClassName, MemberName, __VA_ARGS__), TArgs&&... Args);\
    };\
\
    __VA_OPT__(CREATE_OVERLOAD_HELPER(ClassName, MemberName, ReturnType, __VA_ARGS__))\
    template struct TAccessPrivateFunction<GET_FUNCTION_TAG_TYPE(ClassName, MemberName, __VA_ARGS__), ClassName, ReturnType,\
    __VA_OPT__(static_cast<std::remove_pointer_t<decltype(PRIVATE_CAT(_GetType##ClassName##MemberName,__LINE__)(&ClassName::MemberName))>> PRIVATE_ACCESS_PARAN_LEFT ) &ClassName::MemberName __VA_OPT__( PRIVATE_ACCESS_PARAN_RIGHT )>;\
}\
namespace ClassName##_Private\
{\
    PRIVATE_VALUE_IF(__VA_OPT__(1), CREATE_CALL_FUNCTION_OVERLOAD(ClassName, MemberName, ReturnType, __VA_ARGS__),\
    CREATE_CALL_FUNCTION(ClassName, MemberName, ReturnType, __VA_ARGS__))\
}\


#define DEFINE_PRIVATE_TYPE_ACCESSOR(Type1, Type2) \
namespace BMPrivateAccess\
{\
struct Type1##_##Type2##_##_PrivateAccessTag\
{\
    friend consteval auto* ResolvePrivateType(Type1##_##Type2##_##_PrivateAccessTag);\
};\
\
template struct TAccessPrivateType<Type1##_##Type2##_##_PrivateAccessTag, Type1::Type2>;\
}\
using Type1##_##Type2##_Type = std::remove_pointer_t<decltype(ResolvePrivateType(BMPrivateAccess::Type1##_##Type2##_##_PrivateAccessTag{}))>;


namespace BMPrivateAccess
{
    template<auto TFunctionPtr, typename... TArgs>
    struct TGetFunctionReturnType
    {
        using Type = std::invoke_result_t<decltype(TFunctionPtr), TArgs...>;
    };

    template<typename... TArgs>
    struct TOverloadHelper
    {
        template<typename... TOverloadArgs>
        static constexpr bool bSame = std::same_as<std::tuple<std::decay_t<TArgs>...>, std::tuple<std::decay_t<TOverloadArgs>...>>;
    };
    
    template<auto TFunction, typename... TArgs>
    concept CIsStaticFunctionPtr = requires(TArgs&&... Args)
    {
        {TFunction(std::forward<TArgs>(Args)...)};
    };

    template<auto TMemberPtr>
    concept CIsMemberPtr = requires()
    {
        {*TMemberPtr};
    };

    template<typename Tag, auto TMemberPtr>
    struct TAccessPrivateMember
    {
        template<typename... TArgs>
        friend auto& AccessPrivate(Tag, TArgs&&... Args)
        {
            if constexpr(BMPrivateAccess::CIsMemberPtr<TMemberPtr>)
            {
                return *TMemberPtr;
            }
            else
            {
                auto Helper = [](auto&& Object, auto&&...) -> auto&
                {
                    return Object.*TMemberPtr;
                };

                return Helper(std::forward<TArgs>(Args)...);
            }
        }
    };

    template<typename Tag, typename T, typename TReturn, auto TFunctionPtr>
    struct TAccessPrivateFunction
    {
        template<typename... TArgs>
        friend TReturn CallPrivate(Tag, TArgs&&... Args)
        {
            if constexpr(BMPrivateAccess::CIsStaticFunctionPtr<TFunctionPtr, TArgs...>)
            {
                return TFunctionPtr(std::forward<TArgs>(Args)...);
            }
            else
            {
                auto CallHelper = []<typename TObject, typename... THelperArgs>(TObject&& Object, THelperArgs&&... HelperArgs) -> TReturn
                {
                    return (Object.*TFunctionPtr)(std::forward<THelperArgs>(HelperArgs)...);
                };

                return CallHelper(std::forward<TArgs>(Args)...);
            }
        }
    };

    template<typename Tag, typename T>
    struct TAccessPrivateType
    {
        friend consteval auto* ResolvePrivateType(Tag)
        {
            T* Ptr = nullptr;
            return Ptr;
        }
    };
    
    namespace Danger
    {
        /*
         * DANGER: This function swaps the vtable of the provided object with the vtable of the parent object for the duration of the callable
         * - This is UB and only works on compilers that place the vtable pointer at the start of the object
         * - During the call to any parent functions, the object will be treated as the parent object
         * - Executing any virtual functions on the object will call the parent function
         * - Calling any virtual function not present in the parent object will result in a crash or worse
         */
        template<typename TThis, typename TParent, typename TCallable>
        void SwapVTable(TThis&& ThisObj, TParent&& ParentObj, const TCallable& Callable)
        {
            using TThisType = std::decay_t<TThis>;
            using TParentType = std::decay_t<TParent>;
            static_assert(std::is_base_of_v<TParentType, TThisType>, "TThis must be derived from TParent");

            void*** ThisVTable = (void***)&ThisObj;
            void** ThisVTableValue = *ThisVTable;
            void*** ParentVTable = (void***)&ParentObj;

            *ThisVTable = *ParentVTable;
            Callable();
            *ThisVTable = ThisVTableValue;
        }
    }
}
