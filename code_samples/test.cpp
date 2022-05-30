#include <iostream>
#include <cstdint>
using i = std::uint64_t;
template<i N>
constexpr i fib()
{
	return fib<N - 1>() + fib<N - 2>();
}
template<>
constexpr i fib<1>()
{
	return 1;
}
template<>
constexpr i fib<0>()
{
	return 1;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char ** argv)
{
	using namespace std;
	constexpr auto f = fib<36>();
	cout << "Hello, World!\n" << f << std::endl;
	return 0;
}
