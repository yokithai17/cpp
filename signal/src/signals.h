#pragma once

#include "intrusive_list.h"

#include <functional>
#include <iostream>

namespace signals {

template <typename T>
class signal;

template <typename... Args>
class signal<void(Args...)> {
public:
  using slot_t = std::function<void(Args...)>;

  class connection : public intrusive::list_element<struct connection_tag> {
  public:
    signal* _sig{nullptr};
    slot_t _slot;

    connection(signal* sig, slot_t slt)
        : _sig(sig)
        , _slot(std::move(slt)) {
      _sig->_connections.push_back(*this);
    }

    connection() noexcept = default;

    connection(connection&& other) noexcept
        : _sig(other._sig)
        , _slot(std::move(other._slot)) {
      this->move_helper(other);
    }

    void move_helper(connection& other) {
      other.replace(this);
      other.disconnect();
    }

    connection& operator=(connection&& other) noexcept {
      if (this != &other) {
        this->disconnect();
        _sig = other._sig;
        _slot = std::move(other._slot);
        this->move_helper(other);
      }
      return *this;
    }

    ~connection() {
      disconnect();
    }

    void disconnect() noexcept {
      if (!_sig) {
        return;
      }

      for (auto* token = _sig->_token; token != nullptr; token = token->prev) {
        if (std::addressof(*token->it) == this) {
          ++token->it;
        }
      }

      this->unlink();
      _sig = nullptr;
    }
  };

  using connections_list = intrusive::list<connection, struct connection_tag>;
  using iterator_type = typename connections_list::const_iterator;

  struct iterator_token {
    explicit iterator_token(signal* sig_)
        : sig(sig_)
        , it(sig->_connections.begin())
        , prev(sig->_token) {
      sig->_token = this;
    }

    ~iterator_token() {
      if (sig) {
        sig->_token = prev;
      }
    }

    signal* sig{nullptr};
    iterator_type it;
    iterator_token* prev{nullptr};
    bool is_deleted{false};
  };

  signal() noexcept = default;

  signal(const signal&) = delete;
  signal& operator=(const signal&) = delete;

  signal(signal&& other) noexcept
      : _connections(std::move(other._connections))
      , _token(other._token) {
    invalidate_iterator(&other);
    other._token = nullptr;
  }

  void invalidate_iterator(signal* compare_to) noexcept {
    if (_token) {
      _token->sig = static_cast<signal*>(this);
      if (_token->it == compare_to->_connections.end()) {
        _token->it = _connections.end();
      }
    }
  }

  signal& operator=(signal&& other) noexcept {
    if (this != &other) {
      _connections = std::move(other._connections);
      _token = other._token;
      invalidate_iterator(&other);
      other._token = nullptr;
    }
    return *this;
  }

  ~signal() {
    for (auto* token = _token; token != nullptr; token = token->prev) {
      token->is_deleted = true;
    }

    _connections.clear();
  }

  template <std::invocable<Args...> slot>
  [[nodiscard]] connection connect(slot slt) noexcept {
    return connection(this, std::move(slt));
  }

  void operator()(Args... args) const {
    iterator_token token(const_cast<signal*>(this));

    while (token.it != token.sig->_connections.end()) {
      auto last_end = token.sig->_connections.end();
      auto copy = token.it++;
      copy->_slot(std::forward<Args>(args)...);
      if (token.is_deleted || token.it == last_end) {
        return;
      }
    }
  }

private:
  connections_list _connections{};
  mutable iterator_token* _token{nullptr};
};

} // namespace signals
