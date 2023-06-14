/**
 * @brief 配列サイズ取得
 * 
 * @tparam T Type of array
 * @param arr Target array
 * @return int Array size
 */
template <typename T> int array_size(const T &arr) {
    return sizeof(T) / sizeof(*arr);
};