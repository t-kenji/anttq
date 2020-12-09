/** @file   utils.hpp
 *  @brief  Unit-test utilities.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2019-02-03 newly created.
 */

#ifndef __ANTTQ_TEST_UTILS_H__
#define __ANTTQ_TEST_UTILS_H__

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

template<typename First, typename ...Rest>
constexpr std::string tags(const First first, const Rest ...rest)
{
    const First args[] = {first, rest...};
    std::string tag_str = "";
    for (size_t i = 0; i < sizeof(args)/sizeof(args[0]); ++i) {
        tag_str += "[" + std::string(args[i]) + "]";
    }
    return tag_str;
}

/**
 *  @sa https://stackoverflow.com/a/33047781
 */
struct Lambda {
    template<typename Tret, typename T, typename... Targs>
    static Tret lambda_ptr_exec(Targs... args)
    {
        return (Tret) (*(T *)fn<T>())(args...);
    }

    template<typename Tret, typename... Targs, typename Tfp = Tret(*)(Targs...), typename T>
    static Tfp cify(T& t)
    {
        fn<T>(&t);
        return (Tfp) lambda_ptr_exec<Tret, T, Targs...>;
    }

    template<typename T>
    static void *fn(void *new_fn = nullptr)
    {
        static void *fn;
        if (new_fn != nullptr) {
            fn = new_fn;
        }
        return fn;
    }
};

/**
 *  @sa https://github.com/dascandy/hippomocks/blob/master/HippoMocks/hippomocks.h
 */
#include <cstring>
#include <sys/mman.h>

template <typename T>
class ReplIt
{
    private:
        typedef unsigned int e9ptrsize_t;

        template <typename U, typename V>
        U horrible_cast(V v)
        {
            union { U u; V v; } un;
            un.v = v;
            return un.u;
        }

        class Unprotect
        {
            private:
                intptr_t location_;
                size_t count_;

            public:
                Unprotect(void *location, size_t count): location_((intptr_t)location & ~(0xFFF)),
                                                         count_(count + ((intptr_t)location - location_))
                {
                    mprotect((void *)location_, count_, PROT_READ | PROT_WRITE | PROT_EXEC);
                }

                ~Unprotect()
                {
                    mprotect((void *)location_, count_, PROT_READ | PROT_EXEC);
                }
        };

        T original_;
        uint8_t backup[16]; // typical use is 5 for 32-bit and 14 for 64-bit code.

    public:
        ReplIt(T original, T replacement): original_(original)
        {
#if defined(__x86_64__)
            if (llabs((int64_t)original_ - (int64_t)replacement) < (1LL << 31)) {
                Unprotect _((void *)original_, sizeof(backup));
                memcpy(backup, horrible_cast<void *>(original_), sizeof(backup));

                *(uint8_t *)original_ = 0xE9;
                *(e9ptrsize_t *)((intptr_t)(original_) + 1) =
                    (e9ptrsize_t)((intptr_t)(replacement) - (intptr_t)(original_) - sizeof(e9ptrsize_t) - 1);
            } else {
                Unprotect _((void *)original_, sizeof(backup));
                memcpy(backup, horrible_cast<void *>(original_), sizeof(backup));

                uint8_t *ptr = (uint8_t *)original_;
                ptr[0] = 0xFF; // jmp (rip + imm32)
                ptr[1] = 0x25;
                ptr[2] = 0x00; // imm32 of 0, so immediately after the instruction
                ptr[3] = 0x00;
                ptr[4] = 0x00;
                ptr[5] = 0x00;
                *(intptr_t *)((intptr_t)original_ + 6) = (intptr_t)replacement;
            }
#else
#error "Archtecture is not supported"
#endif
        }

        ~ReplIt()
        {
            Unprotect _((void *)original_, sizeof(backup));
            memcpy(horrible_cast<void *>(original_), backup, sizeof(backup));
        }
};

/**
 *  @sa https://gcc.gnu.org/bugzilla/show_bug.cgi?id=25509#c29
 */
template<typename T>
inline T ignore_result(T x [[gnu::unused]])
{
    return x;
}

int msleep(long msec);
int64_t getuptime(int64_t base);
ssize_t file_read(const char *path, void *buf, size_t count);
ssize_t file_read(std::string path, void *buf, size_t count);
ssize_t file_write(const char *path, void *buf, size_t count);
ssize_t file_write(std::string path, void *buf, size_t count);

#endif // __ANTTQ_TEST_UTILS_H__
