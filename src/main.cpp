#include <iostream>
#include <bitset>
#include <algorithm>
#include <array>
#include <type_traits>
#include <bit>
#include <sstream>
#include <ranges>
#include <vector>

template <typename T>
concept TriviallyCopyable = requires (T) {std::is_trivially_copyable_v<T>;};

namespace byte_conversion
{
	namespace _impl
	{
		template <TriviallyCopyable T>
		constexpr auto direct(const T &value) noexcept
		{
			std::array<std::byte, sizeof(T)> bytes;
			auto start = reinterpret_cast<std::byte const*>(&value);
			std::copy(start, start + sizeof(T), bytes.data());
			return bytes;
		}

		template <TriviallyCopyable T>
		constexpr auto reverse(const T &value) noexcept
		{
			std::array<std::byte, sizeof(T)> bytes;
			auto start = reinterpret_cast<std::byte const*>(&value);
			std::reverse_copy(start, start + sizeof(T), bytes.data());
			return bytes;
		}
	}

	namespace native
	{
		template <TriviallyCopyable T>
		constexpr auto to_bytes(T value) noexcept
		{
			return _impl::direct(value);
		}
	}

	namespace little_endian
	{
		template <TriviallyCopyable T>
		constexpr auto to_bytes(T value) noexcept
		{
			bool direct = std::endian::native ==  std::endian::little;
			return direct ? _impl::direct(value) : _impl::reverse(value);
		}
	}

	namespace big_endian
	{
		template <TriviallyCopyable T>
		constexpr auto to_bytes(T value) noexcept
		{
			bool direct = std::endian::native ==  std::endian::big;
			return direct ? _impl::direct(value) : _impl::reverse(value);
		}
	}

	using namespace native;
}

struct join: std::ranges::range_adaptor_closure<join>
{
	std::string sep{};

	join () = default;
	join( std::string_view sep) : sep{sep}
	{

	}

	template <std::ranges::range Range>
	std::string operator()(const Range &r)
	{
		namespace rng = std::ranges;
		auto end =  rng::end(r);
		std::stringstream ss;
		auto it = rng::begin(r);
		if (it != end)
		{
			auto prev = *it;
			it++;
			while (it != end)
			{
				ss << prev << sep;
				prev = *it;
				it++;
			}
			ss << prev;
		}
		return ss.str();
	}
};

int main()
{
	float f = 1.0 / 3.0;
	auto f_bytes = byte_conversion::to_bytes(f);

	std::vector<std::string> v = {"hello", "world", "this is", "PrettyCool"};

	std::string_view sep = "..";
	std::cout << (v | join{sep}) << std::endl;

	std::vector<int> vi = {10, 20, 30};
	std::cout << (vi | std::views::transform([](int x) { return std::to_string(x);}) | join{sep}) << std::endl;

	std::cout << "float\n";
	for (const auto &b: f_bytes)
	{
		std::cout << std::bitset<8>(static_cast<int>(b)) << ' ';
	}
	std::cout << '\n';

	double d = 1.0 / 3.0;
	auto d_bytes = byte_conversion::to_bytes(d);

	std::cout << "double\n";
	for (auto &b: d_bytes)
	{
		std::cout << std::bitset<8>(static_cast<int>(b)) << ' ';
	}
	std::cout << std::endl;
}
