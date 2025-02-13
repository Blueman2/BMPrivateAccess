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


#define DEFINE_PRIVATE_FUNCTION_ACCESSOR(ClassName, MemberName, ReturnType) \
namespace BMPrivateAccess\
{\
    struct ClassName##MemberName##Tag\
    {\
        template<typename... TArgs>\
        friend ReturnType CallPrivate(ClassName##MemberName##Tag, TArgs&&... Args);\
    };\
    template struct TAccessPrivateFunction<ClassName##MemberName##Tag, ClassName, ReturnType, &ClassName::MemberName>;\
}\
namespace ClassName##_Private\
{\
    template<typename... TArgs>\
    static ReturnType Call_##MemberName(TArgs&&... Args)\
    {\
        return CallPrivate(BMPrivateAccess::ClassName##MemberName##Tag{}, std::forward<TArgs>(Args)...);\
    }\
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
