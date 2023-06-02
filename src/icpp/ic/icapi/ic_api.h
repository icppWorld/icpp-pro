// API functions to be called by a C++ application to interact with ic0
// functions
//
// References:
//  -
//  https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
//  - https://github.com/dfinity/candid/blob/master/spec/Candid.md#binary-format

#pragma once

#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "candid.h"
#include "candid_type_principal.h"
#include "vec_bytes.h"

class IC_API {
public:
  IC_API();
  // docs start: ac_api
  IC_API(const bool &debug_print); // docs end: ac_api

  ~IC_API();

  // docs start: demo_debug_print
  static void debug_print(const char *msg);
  static void debug_print(const std::string &msg); // docs end: demo_debug_print

  // docs start: demo_trap
  static void trap(const char *msg);
  static void trap(const std::string &msg); // docs end: demo_trap

  // docs start: demo_time
  uint64_t time(); // docs end: demo_time

  // docs start: get_caller
  CandidTypePrincipal get_caller(); // docs end: get_caller

  // Receive things from the wire in candid format
  // docs start: from_wire
  void from_wire();
  void from_wire(CandidType arg_in);
  void from_wire(std::vector<CandidType> args_in); // docs end: from_wire

  // Convert string to __uint128_t & __int128_t
  // clang-format off
  // docs start: string_to_128
  static std::optional<__uint128_t> string_to_uint128_t(const std::string &str);
  static std::optional<__int128_t> string_to_int128_t(const std::string &str); // docs end: string_to_128
  // clang-format on

  // Convert __uint128_t & __int128_t to std::string
  // docs start: to_string_128
  static std::string to_string_128(__uint128_t v);
  static std::string to_string_128(__int128_t v); // docs end: to_string_128

  // Send things back over the wire in candid format
  // docs start: to_wire
  void to_wire();
  void to_wire(const CandidType &arg_out);
  void to_wire(const std::vector<CandidType> &args_out); // docs end: to_wire

  template <typename T>
  void store_vector_orthogonal(const std::vector<T> &vec, T **p_data,
                               __uint128_t *len) {
    persist_container(vec, p_data, len);
  }

  template <typename T>
  void store_list_orthogonal(const std::list<T> &lst, T **p_data,
                             __uint128_t *len) {
    persist_container(lst, p_data, len);
  }

  template <typename T>
  void store_deque_orthogonal(const std::deque<T> &dq, T **p_data,
                              __uint128_t *len) {
    persist_container(dq, p_data, len);
  }

  template <typename T, size_t N>
  void store_array_orthogonal(const std::array<T, N> &arr, T **p_data,
                              __uint128_t *len) {
    persist_container(arr, p_data, len);
  }

  template <typename Key, typename T>
  void store_map_orthogonal(const std::map<Key, T> &mp,
                            std::pair<const Key, T> **p_data,
                            __uint128_t *len) {
    persist_container(mp, p_data, len);
  }

  template <typename Key, typename T>
  void store_unordered_map_orthogonal(const std::unordered_map<Key, T> &ump,
                                      std::pair<const Key, T> **p_data,
                                      __uint128_t *len) {
    persist_container(ump, p_data, len);
  }

  template <typename T>
  void store_set_orthogonal(const std::set<T> &st, T **p_data,
                            __uint128_t *len) {
    persist_container(st, p_data, len);
  }

  template <typename T>
  void store_unordered_set_orthogonal(const std::unordered_set<T> &ust,
                                      T **p_data, __uint128_t *len) {
    persist_container(ust, p_data, len);
  }

  void store_string_orthogonal(const std::string &str, char **p_data,
                               __uint128_t *len) {
    persist_container(str, p_data, len);
  }

  template <typename T>
  std::vector<T> retrieve_vector_orthogonal(const T *p_data, __uint128_t len) {
    return std::vector<T>(p_data, p_data + len);
  }

  template <typename T>
  std::list<T> retrieve_list_orthogonal(const T *p_data, __uint128_t len) {
    return std::list<T>(p_data, p_data + len);
  }

  template <typename T>
  std::deque<T> retrieve_deque_orthogonal(const T *p_data, __uint128_t len) {
    return std::deque<T>(p_data, p_data + len);
  }

  template <typename T, size_t N>
  std::array<T, N> retrieve_array_orthogonal(const T *p_data, __uint128_t len) {
    std::array<T, N> arr;
    std::copy(p_data, p_data + std::min(len, static_cast<__uint128_t>(N)),
              arr.begin());
    return arr;
  }

  template <typename Key, typename T>
  std::map<Key, T>
  retrieve_map_orthogonal(const std::pair<const Key, T> *p_data,
                          __uint128_t len) {
    return std::map<Key, T>(p_data, p_data + len);
  }

  template <typename Key, typename T>
  std::unordered_map<Key, T>
  retrieve_unordered_map_orthogonal(const std::pair<const Key, T> *p_data,
                                    __uint128_t len) {
    return std::unordered_map<Key, T>(p_data, p_data + len);
  }

  template <typename T>
  std::set<T> retrieve_set_orthogonal(const T *p_data, __uint128_t len) {
    return std::set<T>(p_data, p_data + len);
  }

  template <typename T>
  std::unordered_set<T> retrieve_unordered_set_orthogonal(const T *p_data,
                                                          __uint128_t len) {
    return std::unordered_set<T>(p_data, p_data + len);
  }

  std::string retrieve_string_orthogonal(const char *p_data, __uint128_t len) {
    return std::string(p_data, p_data + len);
  }

private:
  bool m_debug_print;
  VecBytes m_B_in;
  VecBytes m_B_out;
  CandidTypePrincipal m_caller;
  void msg_reply();

  bool m_called_from_wire{false};
  bool m_called_to_wire{false};

  template <typename Container>
  void persist_container(const Container &container,
                         typename Container::value_type **p_data,
                         __uint128_t *len) {
    if (*p_data) {
      delete[] * p_data;
    }
    *len = container.size();
    *p_data = new typename Container::value_type[*len];
    if (*p_data == nullptr) {
      IC_API::trap("persist_container: Out of memory!");
    }
    std::copy(container.begin(), container.end(), *p_data);
  }
};