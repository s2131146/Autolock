#ifndef AUTOLOCK_ARRAY_H_
#define AUTOLOCK_ARRAY_H_

/**
 * @brief 要素をシフト
 * 
 * @tparam T 
 * @tparam N 
 * @param arr 
 */
template <typename T, unsigned int N> void shift(T (&arr)[N]) {
    if (N == 0) return;
    T tmp = arr[0];
    for (int i = 0; i < N - 1; i++) { arr[i] = arr[i + 1]; }
    arr[N - 1] = tmp;
}

#endif