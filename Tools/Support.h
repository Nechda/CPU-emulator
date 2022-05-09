#pragma once

struct Pointer {
    Pointer() : raw_(nullptr) {}
    template<typename T>
    Pointer(T* ptr) : raw_(ptr) {}
    char* AsByte() const { return reinterpret_cast<char*>(raw_); }
    template<typename T>
    T* As() const { return reinterpret_cast<T*>(raw_); }
    template<typename T>
    T& DrefAs() const { return *reinterpret_cast<T*>(raw_); }
    Pointer& operator += (long diff) {
        char* r = (char*)raw_;
        r += diff;
        raw_ = r;
    }
    Pointer& operator -= (long diff) {
        char* r = (char*)raw_;
        r -= diff;
        raw_ = r;
    }
  private:
    void* raw_;
};

template<typename T1, typename T2> 
struct PairHolder {
    PairHolder(T1& a, T2& b) : a_{a}, b_{b} {}
    template<typename T>
    void operator+=(const T& rhs) && {
        a_ += rhs;
        b_ += rhs;
    }
private:
    T1& a_;
    T2& b_;
};