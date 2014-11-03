/*
spatial - spatial search acceleration using morton codes
Written in 2014 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <cassert>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <array>
#include <map>

std::random_device rd;
std::default_random_engine generator(rd());
std::uniform_real_distribution<double> distribution(-1.0f, 1.0f);
auto myrand = std::bind(distribution, generator);

template<typename T> struct uintof;
template<> struct uintof<float> { typedef uint32_t type; };
template<> struct uintof<double> { typedef uint64_t type; };

template<typename T>
struct aabb {
	T min, max;
};

template<typename T, std::size_t O>
struct grid {
	typedef typename T::value_type value_type;
	typedef typename uintof<value_type>::type uint_t;
	static constexpr std::size_t order = O;
	static constexpr value_type length = 1 << order;
	aabb<T> box;
	uint_t morton(const T &point)
	{
		assert(box.min.size() == box.max.size());
		assert(box.min.size() == point.size());
		assert(order * point.size() <= 8 * sizeof(uint_t));
		uint_t code(0);
		for (std::size_t i = 0; i < point.size(); ++i) {
			const uint_t voxel = length * ((point[i] - box.min[i]) / (box.max[i] - box.min[i]));
			uint_t tmp(0);
			for (std::size_t j = 0; j < order; ++j)
				tmp |= ((uint_t(1) << j) & voxel) << j * (point.size()-1);
			code |= tmp << i;
		}
		return code;
	}
};

template<typename T>
static inline typename T::value_type distance(const T &a, const T &b)
{
	assert(a.size() == b.size());
	typename T::value_type sum(0);
	for (std::size_t i = 0; i < a.size(); ++i)
		sum += (a[i] - b[i]) * (a[i] - b[i]);
	using std::sqrt;
	return sqrt(sum);
}

int main()
{
	typedef std::array<float, 3> vector;
	grid<vector, 10> grid;

	for (std::size_t i = 0; i < grid.box.min.size(); ++i) {
		grid.box.min[i] = -1;
		grid.box.max[i] = 1;
	}

	typedef std::pair<uintof<vector::value_type>::type, vector> leaf;
	auto map = new std::array<leaf, 100000>;
	auto start = std::chrono::system_clock::now();
	for (auto &e: *map) {
		vector point;
		for (auto &c: point)
			c = myrand();
		e.first = grid.morton(point);
		e.second = point;
	}
	std::sort(map->begin(), map->end(), [](const leaf &a, const leaf &b){ return a.first < b.first; });
	auto end = std::chrono::system_clock::now();
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "insertion of " << map->size() << " random points took " << msec.count() << " milliseconds." << std::endl;

	start = std::chrono::system_clock::now();
	int num = 10000000;
	int found = 0, match = 0;
	for (int i = 0; i < num; ++i) {
		vector point;
		for (auto &c: point)
			c = myrand();
		auto code = grid.morton(point);
		auto lb = lower_bound(map->begin(), map->end(), code, [](const leaf &a, const decltype(code) &b) -> bool { return a.first < b; });
		for (auto it = lb; it != map->end() && it->first == code; ++it, ++found)
			match += distance(it->second, point) < 0.001;
	}
	end = std::chrono::system_clock::now();
	msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "searching for " << num << " random points took " << msec.count() << " milliseconds." << std::endl;
	std::cout << found << " voxels found, " << match << " close matching points." << std::endl;
	return 0;
}
