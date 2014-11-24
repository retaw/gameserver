#ifndef WATER_BASE_COMPONENT_CLASS_HELPER_H
#define WATER_BASE_COMPONENT_CLASS_HELPER_H

#include <memory>

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

#define STR(str) #str
#define MACRO_2_STR(str) STR(str)

#define TYPEDEF_PTR(ClassName) \
typedef std::shared_ptr<ClassName> Ptr;\
typedef std::shared_ptr<const ClassName> CPtr;\
typedef std::weak_ptr<ClassName> WPtr;\
typedef std::weak_ptr<const ClassName> CWPtr;

#define CREATE_FUN_NEW(ClassName)\
template <typename ... ArgsType>\
static std::shared_ptr<ClassName> create(ArgsType&& ... args)\
{\
    return std::shared_ptr<ClassName>(new ClassName(std::forward<ArgsType>(args)...));\
}

#define CREATE_FUN_MAKE(ClassName)\
template <typename ... ArgsType>\
static std::shared_ptr<ClassName> create(ArgsType&& ... args)\
{\
    return std::make_shared<ClassName>(std::forward<ArgsType>(args)...);\
}

#define NON_COPYABLE(ClassName) \
ClassName(const ClassName&) = delete; \
ClassName& operator=(const ClassName&) = delete;




#endif
