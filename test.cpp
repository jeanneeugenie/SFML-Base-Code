#include <iostream>
int main() {
#if __cplusplus >= 202002L
	std::cout << " Compiling as C++20 (value: " << __cplusplus << ")\n";
#else
	std::cout << " Not actually C++20 (__cplusplus=" << __cplusplus << ")\n";
#endif
}
