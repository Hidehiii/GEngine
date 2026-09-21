// Include the public logging header first, without relying on an engine PCH.
#include <GEngine/Core/Log.h>
#include <stdexcept>

#if defined(_MSC_VER) && defined(_DEBUG)
static_assert(_ITERATOR_DEBUG_LEVEL == 2, "Debug iterator checks must remain enabled.");
#endif

int main()
{
    const auto message = fmt::format("GEngine {} {} {:.2f}", 2026, "logging", 1.25);
    if (message != "GEngine 2026 logging 1.25")
        throw std::runtime_error("Logging formatting regression.");
    return 0;
}
