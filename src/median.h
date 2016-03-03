#ifndef _MEDIAN_H_
#define _MEDIAN_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#include <queue>


unsigned inline keep_odd(unsigned n)
{
    if(n % 2 == 0) return n + 1;
    return n;
}

template <class T>
class MedianFilter
{
    std::vector<T> _history;
    std::deque<T>  _pool;
    unsigned       _median;
	unsigned		_hist_ptr;

public:

    MedianFilter(unsigned window_size)
        :
        _history(keep_odd(window_size), T()),
        _pool(keep_odd(window_size), T()),
        _median(window_size / 2 + 1),
		_hist_ptr(0)
    {}

	T push(const T &in)
	{
		if(_history.size() == 0) {
			std::fill(_history.begin(), _history.end(), in);
			std::fill(_pool.begin(), _pool.end(), in);
			return in;
		}
		// step 1, remove oldest value from the pool.

		auto last = _history[_hist_ptr];

		auto last_pos = std::lower_bound(_pool.begin(), _pool.end(), last);

		_pool.erase(last_pos);

		// step 2, insert new value into pool

		auto insert_pos = std::lower_bound(_pool.begin(), _pool.end(), in);

		_pool.insert(insert_pos, in);

		// step 3, write input value into history.

		_history[_hist_ptr] = in;

		_hist_ptr = (_hist_ptr + 1) % _history.size();

		// median is always the middle of the pool

		return (*(_pool.begin() +_median));

	}

};


#endif

#endif /* _MEDIAN_H_ */

