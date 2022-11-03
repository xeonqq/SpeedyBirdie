namespace math {

template <typename T, typename U>
T map(T x, T in_min, T in_max, U out_min, U out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
} // namespace math
