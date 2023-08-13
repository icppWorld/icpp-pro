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
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "canister.h"
#include "vec_bytes.h"

class IC_API {
public:
  IC_API();

  [[deprecated("Replace with: IC_API ic_api(CanisterQuery{std::string(__func__)}, false)")]] IC_API(
      const bool &debug_print);

  // docs start: ic_api
  IC_API(const CanisterBase &canister_entry,
         const bool &debug_print); // docs end: ic_api

  ~IC_API();

  // docs start: demo_debug_print
  static void debug_print(const char *msg);
  static void debug_print(const std::string &msg); // docs end: demo_debug_print

  // docs start: demo_trap
  static void trap(const char *msg);
  static void trap(const std::string &msg); // docs end: demo_trap

  // docs start: demo_time
  static uint64_t time(); // docs end: demo_time

  // docs start: get_caller
  CandidTypePrincipal get_caller(); // docs end: get_caller

  // Receive things from the wire in candid format
  // docs start: from_wire
  void from_wire();
  void from_wire(CandidType arg_in);
  void from_wire(CandidArgs args_in); // docs end: from_wire

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
  void to_wire(const CandidArgs &args_out); // docs end: to_wire

  // Orthogonal Persistence
  template <typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_vector_orthogonal(const std::vector<T> &vec, T **p_data,
                          __uint128_t *len) {
    persist_container_sequence(vec, p_data, len, typeid(vec).name());
  }

  template <typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_list_orthogonal(const std::list<T> &lst, T **p_data, __uint128_t *len) {
    persist_container_sequence(lst, p_data, len, typeid(lst).name());
  }

  template <typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_deque_orthogonal(const std::deque<T> &dq, T **p_data,
                         __uint128_t *len) {
    persist_container_sequence(dq, p_data, len, typeid(dq).name());
  }

  template <typename T, size_t N>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_array_orthogonal(const std::array<T, N> &arr, T **p_data,
                         __uint128_t *len) {
    persist_container_sequence(arr, p_data, len, typeid(arr).name());
  }

  template <typename Key, typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_map_orthogonal(const std::map<Key, T> &mp, std::pair<Key, T> **p_data,
                       __uint128_t *len) {
    persist_container_associative(mp, p_data, len, typeid(mp).name());
  }

  template <typename Key, typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_unordered_map_orthogonal(const std::unordered_map<Key, T> &ump,
                                 std::pair<Key, T> **p_data, __uint128_t *len) {
    persist_container_associative(ump, p_data, len, typeid(ump).name());
  }

  template <typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_set_orthogonal(const std::set<T> &st, T **p_data, __uint128_t *len) {
    persist_container_sequence(st, p_data, len, typeid(st).name());
  }

  template <typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_unordered_set_orthogonal(const std::unordered_set<T> &ust, T **p_data,
                                 __uint128_t *len) {
    persist_container_sequence(ust, p_data, len, typeid(ust).name());
  }

  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  store_string_orthogonal(const std::string &str, char **p_data) {
    delete[] *p_data; // Delete previous memory.
    // Allocate enough memory for the string and null terminator.
    size_t len = str.size() + 1;
    *p_data = new char[len];
    std::copy(str.begin(), str.end(), *p_data);
    (*p_data)[len] = '\0'; // Add null terminator.
  }

  template <typename T>
  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::vector<T>
  retrieve_vector_orthogonal(const T *p_data, __uint128_t len) {
    return std::vector<T>(p_data, p_data + len);
  }

  template <typename T>
  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::list<T>
  retrieve_list_orthogonal(const T *p_data, __uint128_t len) {
    return std::list<T>(p_data, p_data + len);
  }

  template <typename T>
  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::deque<T>
  retrieve_deque_orthogonal(const T *p_data, __uint128_t len) {
    return std::deque<T>(p_data, p_data + len);
  }

  template <typename T, size_t N>
  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::array<T, N>
  retrieve_array_orthogonal(const T *p_data, __uint128_t len) {
    std::array<T, N> arr;
    std::copy(p_data, p_data + std::min(len, static_cast<__uint128_t>(N)),
              arr.begin());
    return arr;
  }

  template <typename Key, typename T>
  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::map<Key, T>
  retrieve_map_orthogonal(const std::pair<Key, T> *p_data, __uint128_t len) {
    std::map<Key, T> result;
    for (__uint128_t i = 0; i < len; ++i) {
      result.insert({p_data[i].first, p_data[i].second});
    }
    return result;
  }

  template <typename Key, typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::
      unordered_map<Key, T>
      retrieve_unordered_map_orthogonal(const std::pair<Key, T> *p_data,
                                        __uint128_t len) {
    std::unordered_map<Key, T> result;
    for (__uint128_t i = 0; i < len; ++i) {
      result.insert({p_data[i].first, p_data[i].second});
    }
    return result;
  }

  template <typename T>
  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::set<T>
  retrieve_set_orthogonal(const T *p_data, __uint128_t len) {
    return std::set<T>(p_data, p_data + len);
  }

  template <typename T>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::
      unordered_set<T>
      retrieve_unordered_set_orthogonal(const T *p_data, __uint128_t len) {
    return std::unordered_set<T>(p_data, p_data + len);
  }

  [[deprecated(
      "See GitHub: icppWorld/icpp-demos/canisters/memory)")]] std::string
  retrieve_string_orthogonal(const char *p_data) {
    if (p_data == nullptr) {
      return std::string(); // Return empty string if p_data is nullptr.
    }
    return std::string(p_data);
  }

private:
  CanisterBase m_canister_entry;
  bool m_debug_print{false};
  VecBytes m_B_in;
  VecBytes m_B_out;
  CandidTypePrincipal m_caller;
  void msg_reply();

  bool m_called_from_wire{false};
  bool m_called_to_wire{false};

  // For sequence containers (array, list, vec, string)
  template <typename Container>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  persist_container_sequence(const Container &container,
                             typename Container::value_type **p_data,
                             __uint128_t *len,
                             const std::string container_type) {
    if (*p_data) {
      delete[] *p_data;
    }
    *len = container.size();
    *p_data = new typename Container::value_type[*len];
    if (*p_data == nullptr) {
      IC_API::trap("persist_container_sequence: Out of memory when storing " +
                   container_type);
    }
    std::copy(container.begin(), container.end(), *p_data);
  }

  // For associative containers whose data is stored using an a key (map, unordered_map)
  template <typename Container, typename NonConstValueType>
  [[deprecated("See GitHub: icppWorld/icpp-demos/canisters/memory)")]] void
  persist_container_associative(const Container &container,
                                NonConstValueType **p_data, __uint128_t *len,
                                const std::string container_type) {
    if (*p_data) {
      delete[] *p_data;
    }
    *len = container.size();
    *p_data = new NonConstValueType[*len];
    if (*p_data == nullptr) {
      IC_API::trap(
          "persist_container_associative: Out of memory when storing " +
          container_type);
    }
    auto it = container.begin();
    for (uint64_t i = 0; i < *len; ++i, ++it) {
      (*p_data)[i] = {it->first, it->second};
    }
  }
};